#pragma once

#include "cmqc.h"

struct MqApi;  // see MqApi.h — forward-declared to avoid circular include

// P4.1 — passive holder for the queue-manager connection.
//
// PR B added the settings sub-structs (TLS, credentials, heartbeat,
// reconnect config, health monitor config).
//
// PR C (this commit) extends the sub-structs with the per-connection
// **runtime state** that goes with them (reconnect attempt counters,
// health monitor counters, current QM identification) and adds a new
// `current` sub-struct for "what's the current connection" data
// (qm name, user, qm level / platform / ccsid).
//
// DataArea retains reference aliases (same pattern as PR A/B) so
// existing call sites in DataArea.cpp, General.cpp, rfhutilView.cpp,
// ConnSettings.cpp keep compiling unchanged.
//
// Up next:
//   PR D — extract an MqApi struct holding the dynamically-loaded
//          MQ function pointers (XMQConnX, XMQDisc, XMQOpen, ...)
//          currently scattered as DataArea members, then move the
//          method bodies (connect, disconnect, reconnect, health
//          monitor) onto MQConnection.
//   PR E — migrate the ~88 call sites onto m_connection.X.Y style,
//          remove the reference aliases and forwarders, RAII destructor
//          closes the handle (folds in deferred P3.7).
class MQConnection
{
public:
    MQConnection();

    // The handle and connected flag (PR A).
    MQHCONN m_qm;
    bool    m_connected;

    // PR D: pointer to the dynamically-loaded MQ API. Set by DataArea's
    // constructor (m_connection.m_api = &m_api). Stays null in any
    // tests/contexts that construct a bare MQConnection without an
    // api; PR E's method-body migration adds null-guards if needed.
    MqApi* m_api;

    // ─── PR E: small read-only / pure-state methods migrated from DataArea ──
    // These don't touch m_api or the host's log/trace surface, so they're
    // the safest first round of method moves. The bigger methods (connect2QM,
    // discQM, attemptReconnection, performHealthCheck) follow in PR F using
    // onLog/onTrace callbacks and m_api->XMQ*.

    // Was DataArea::isConnectionActive — true while m_connected.
    bool isActive() const;

    // Was DataArea::shouldAttemptReconnect — pure switch on rc.
    bool shouldAttemptReconnect(MQLONG rc) const;

    // Was DataArea::calculateReconnectDelay — exponential backoff math.
    int calculateReconnectDelay() const;

    // Was DataArea::resetReconnectionState — zeroes attempt_count,
    // last_attempt_time, in_progress.
    void resetReconnectionState();

    // Was DataArea::getHealthStatusText / getUptimeText / getLastCheckText —
    // display strings for the Conn-tab health-monitor section.
    CString getHealthStatusText() const;
    CString getUptimeText() const;
    CString getLastCheckText() const;

    // TLS configuration (P3.9).
    struct TlsSettings {
        BOOL    use_ssl;
        BOOL    validate_client;
        CString cipher;
        CString keyr;
        CString peer;
        BOOL    fips_required;
        int     reset_count;
    } tls;

    // User credentials and security exits.
    struct Credentials {
        CString userid;
        CString password;
        CString security_exit;
        CString security_data;
        CString local_address;
    } credentials;

    // P0.1 HeartBeat + KeepAlive.
    struct HeartbeatSettings {
        BOOL heartbeat_enabled;
        int  heartbeat_interval;
        BOOL keepalive_enabled;
        int  keepalive_interval;
    } heartbeat;

    // P0.2 Automatic reconnection — configuration + runtime state.
    struct Reconnect {
        // Settings
        BOOL auto_reconnect;
        int  max_attempts;
        int  interval;
        int  backoff_multiplier;
        int  max_interval;
        // Runtime (PR C)
        int     attempt_count;        // current attempt number
        DWORD   last_attempt_time;    // GetTickCount() of last attempt
        BOOL    in_progress;          // reconnection in progress
        BOOL    connection_was_lost;  // set by notify-connection-lost path
        CString last_qm_name;         // for reconnection
        CString last_channel_name;
        CString last_conn_name;
    } reconnect;

    // P2.1 Health monitor — configuration + runtime state.
    struct HealthMonitor {
        // Settings
        BOOL enabled;
        int  check_interval;
        // Runtime (PR C)
        BOOL   check_active;
        MQHOBJ check_handle;            // cached QM object handle for MQINQ probes
        DWORD  connection_start_time;   // when this connection was established
        DWORD  last_check_time;         // last successful check
        int    check_count;
        int    check_failures;
        int    total_reconnections;     // since app start
        int    status;                  // 0=disconnected, 1=healthy, 2=degraded, 3=reconnecting
    } health;

    // Current connection identification (populated by connect2QM).
    struct CurrentConnection {
        CString qm_name;                // was currentQM
        CString userid;                 // was currentUserid
        MQLONG  level;                  // queue manager level
        MQLONG  platform;               // queue manager platform type
        MQLONG  ccsid;                  // queue manager ccsid (was MQCcsid)
    } current;

    // ─── PR F: disconnect + health-monitor lifecycle ──────────────────────
    // These methods use m_api->XMQ* and touch connection-only state. They
    // do NOT touch DataArea's queue handles, browse state, log/trace
    // surface, or setErrorMsg — the DataArea wrappers (discQM,
    // explicitDiscQM, connectionLostCleanup, startHealthMonitor,
    // stopHealthMonitor) handle those concerns before/after delegating.

    // P2.1 — opens the cached QM handle for MQINQ probes. Returns true
    // if the monitor became active. Sets health.status = 1 on success;
    // leaves it at 0 on MQOPEN failure. No-op (returns false) when
    // !m_connected, already active, or m_api null. cc/rc out-params let
    // the wrapper trace the MQOPEN result.
    bool startHealthMonitor(MQLONG& cc, MQLONG& rc);

    // P2.1 — closes the cached handle if open, clears check_active/status.
    // Best-effort: errors from XMQClose are reported via cc/rc but
    // otherwise ignored.
    void stopHealthMonitor(MQLONG& cc, MQLONG& rc);

    // Disconnect portion of the old discQM(): calls stopHealthMonitor
    // internally, then m_api->XMQDisc if m_connected, then clears
    // connection state (m_qm, m_connected, current.qm_name). The
    // DataArea wrapper still closes any open queue first (queue handles
    // aren't part of MQConnection). cc/rc reflect XMQDisc's result.
    void disconnect(MQLONG& cc, MQLONG& rc);

    // Connection-state portion of the old connectionLostCleanup(): clears
    // m_qm, m_connected, current state, the health monitor's cached
    // handle/counters/status, and marks reconnect.connection_was_lost
    // = TRUE so the next successful connect logs "Reconnected ...". The
    // DataArea wrapper handles queue/browse-state cleanup before calling
    // this.
    void notifyConnectionLost();

    // RAII (folds in deferred P3.7): if still connected when destroyed,
    // attempts XMQDisc to release the handle cleanly. Best-effort —
    // shutdown errors are swallowed since there's no one to surface them
    // to at this point.
    ~MQConnection();

private:
    // No copies / moves — DataArea owns one for its lifetime; copying
    // the handle would lead to double-disconnect on destruction.
    MQConnection(const MQConnection&);
    MQConnection& operator=(const MQConnection&);
};

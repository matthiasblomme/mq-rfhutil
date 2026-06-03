#pragma once

#include "cmqc.h"

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
};

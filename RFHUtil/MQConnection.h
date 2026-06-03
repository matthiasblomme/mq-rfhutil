#pragma once

#include "cmqc.h"

// P4.1 — passive holder for the queue-manager connection.
//
// PR B scope (this commit): connection **settings** move into typed
// sub-structs here. Per-connection *runtime state* (reconnect attempt
// counters, health-check counters, current QM name, etc.) stays on
// DataArea for PR B and moves in PR C alongside the methods that own it.
//
// DataArea retains reference aliases (added in PR A's style) so existing
// read/write code in DataArea.cpp, General.cpp, rfhutilView.cpp,
// ConnSettings.cpp continues to compile unchanged.
//
// Sub-structs:
//   tls          — TLS configuration (cipher, peer name, FIPS, key repo, ...)
//   credentials  — user id / password / security exits / local address
//   heartbeat    — HeartBeat & KeepAlive intervals (P0.1)
//   reconnect    — auto-reconnect configuration (P0.2)
//   health       — health monitor configuration (P2.1)
//
// PR C will move method bodies (connect2QM, discQM, attemptReconnection,
// performHealthCheck, ...) and their runtime state onto this class.
// PR D removes the reference aliases on DataArea and migrates the ~88
// call sites.
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

    // P0.2 Automatic reconnection — configuration only. Runtime counters
    // (attempt_count, last_attempt_time, in_progress, connection_was_lost,
    // last_qm/channel/conn_name) stay on DataArea for PR B and migrate in
    // PR C with attemptReconnection itself.
    struct ReconnectSettings {
        BOOL auto_reconnect;
        int  max_attempts;
        int  interval;
        int  backoff_multiplier;
        int  max_interval;
    } reconnect;

    // P2.1 Health monitor — configuration only. Runtime state (check_active,
    // hHealthCheckObj, counters, status) stays on DataArea for PR B and
    // migrates in PR C with performHealthCheck itself.
    struct HealthMonitorSettings {
        BOOL enabled;
        int  check_interval;
    } health;
};

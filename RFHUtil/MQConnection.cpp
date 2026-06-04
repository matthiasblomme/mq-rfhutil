#include "stdafx.h"
#include "MQConnection.h"

// P4.1 PR B + PR C: defaults for settings (PR B) and runtime state (PR C).
// Values match what DataArea's ctor and ConnSettings's reset paths used.
// CString members default to empty via their own constructors — no explicit
// init needed for those.
MQConnection::MQConnection()
    : m_qm(MQHO_NONE)
    , m_connected(false)
    , m_api(NULL)  // PR D: set by DataArea ctor body
{
    // TLS settings — off / empty by default.
    tls.use_ssl         = FALSE;
    tls.validate_client = FALSE;
    tls.fips_required   = FALSE;
    tls.reset_count     = 0;

    // Credentials — all CStrings, default-constructed empty.

    // HeartBeat / KeepAlive — disabled; intervals 0 = use defaults.
    heartbeat.heartbeat_enabled  = FALSE;
    heartbeat.heartbeat_interval = 0;
    heartbeat.keepalive_enabled  = FALSE;
    heartbeat.keepalive_interval = 0;

    // Auto-reconnect — settings (PR B).
    reconnect.auto_reconnect     = FALSE;
    reconnect.max_attempts       = 3;
    reconnect.interval           = 5;
    reconnect.backoff_multiplier = 2;
    reconnect.max_interval       = 60;
    // Auto-reconnect — runtime state (PR C).
    reconnect.attempt_count       = 0;
    reconnect.last_attempt_time   = 0;
    reconnect.in_progress         = FALSE;
    reconnect.connection_was_lost = FALSE;

    // Health monitor — settings (PR B).
    health.enabled        = FALSE;
    health.check_interval = 30;
    // Health monitor — runtime state (PR C).
    health.check_active           = FALSE;
    health.check_handle           = MQHO_NONE;
    health.connection_start_time  = 0;
    health.last_check_time        = 0;
    health.check_count            = 0;
    health.check_failures         = 0;
    health.total_reconnections    = 0;
    health.status                 = 0;

    // Current-connection state (PR C).
    current.level    = 0;
    current.platform = 0;
    current.ccsid    = 0;
}

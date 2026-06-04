#include "stdafx.h"
#include "MQConnection.h"

// P4.1 PR B: defaults for each settings sub-struct match the values that
// DataArea's constructor and ConnSettings's reset paths used to set.
// CString members default to empty via their own constructor — no explicit
// init needed for those.
MQConnection::MQConnection()
    : m_qm(MQHO_NONE)
    , m_connected(false)
{
    // TLS — off / empty by default. User opts in via the Set Conn Id dialog
    // or via saved registry values restored at startup.
    tls.use_ssl         = FALSE;
    tls.validate_client = FALSE;
    tls.fips_required   = FALSE;
    tls.reset_count     = 0;

    // Credentials sub-struct — CStrings default to empty. Nothing to do.

    // HeartBeat / KeepAlive — disabled; intervals 0 = use defaults.
    heartbeat.heartbeat_enabled  = FALSE;
    heartbeat.heartbeat_interval = 0;
    heartbeat.keepalive_enabled  = FALSE;
    heartbeat.keepalive_interval = 0;

    // Auto-reconnect — pre-PR-B DataArea defaults: 3 attempts, 5s start,
    // backoff x2, cap 60s.
    reconnect.auto_reconnect     = FALSE;
    reconnect.max_attempts       = 3;
    reconnect.interval           = 5;
    reconnect.backoff_multiplier = 2;
    reconnect.max_interval       = 60;

    // Health monitor — disabled; check every 30s when enabled.
    health.enabled        = FALSE;
    health.check_interval = 30;
}

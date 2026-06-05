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

// ─────────────────────────────────────────────────────────────────────────
// PR E: small methods migrated from DataArea. Bodies are byte-identical to
// the pre-move versions except they read from the typed sub-structs
// (reconnect.*, health.*, m_connected) instead of the aliased m_* names.
// DataArea retains forwarder methods so external callers compile unchanged.
// ─────────────────────────────────────────────────────────────────────────

bool MQConnection::isActive() const
{
    return m_connected;
}

bool MQConnection::shouldAttemptReconnect(MQLONG rc) const
{
    // Only attempt reconnection for connection-related errors.
    switch (rc)
    {
    case MQRC_CONNECTION_BROKEN:
    case MQRC_Q_MGR_NOT_AVAILABLE:
    case MQRC_CONNECTION_QUIESCING:
    case MQRC_CONNECTION_STOPPED:
    case MQRC_HCONN_ERROR:
    case MQRC_Q_MGR_STOPPING:
        return true;
    default:
        return false;
    }
}

int MQConnection::calculateReconnectDelay() const
{
    int delay = reconnect.interval;

    // Exponential backoff, capped at max_interval.
    if (reconnect.backoff_multiplier > 1 && reconnect.attempt_count > 0)
    {
        for (int i = 1; i < reconnect.attempt_count; i++)
        {
            delay *= reconnect.backoff_multiplier;
            if (delay > reconnect.max_interval)
            {
                delay = reconnect.max_interval;
                break;
            }
        }
    }

    return delay;
}

void MQConnection::resetReconnectionState()
{
    reconnect.attempt_count     = 0;
    reconnect.last_attempt_time = 0;
    reconnect.in_progress       = FALSE;
}

CString MQConnection::getHealthStatusText() const
{
    switch (health.status)
    {
    case 1: return "Connected (healthy)";
    case 2: return "Connection degraded";
    case 3: return "Reconnecting...";
    default: return "Not connected";
    }
}

CString MQConnection::getUptimeText() const
{
    if (!m_connected || health.connection_start_time == 0)
        return "-";

    DWORD elapsed = (GetTickCount() - health.connection_start_time) / 1000;
    int hours   = elapsed / 3600;
    int minutes = (elapsed % 3600) / 60;
    int seconds = elapsed % 60;

    CString text;
    if (hours > 0)
        text.Format("%dh %dm %ds", hours, minutes, seconds);
    else if (minutes > 0)
        text.Format("%dm %ds", minutes, seconds);
    else
        text.Format("%ds", seconds);
    return text;
}

CString MQConnection::getLastCheckText() const
{
    if (!health.check_active || health.last_check_time == 0)
        return "-";

    DWORD elapsed = (GetTickCount() - health.last_check_time) / 1000;

    CString text;
    if (elapsed < 2)
        text = "Just now";
    else
        text.Format("%d seconds ago", elapsed);
    return text;
}

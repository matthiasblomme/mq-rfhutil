#include "stdafx.h"
#include "MQConnection.h"
#include "MqApi.h"  // PR F: need the full MqApi definition for m_api->XMQ*

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

// ─────────────────────────────────────────────────────────────────────────
// PR F: disconnect and health monitor lifecycle methods. Bodies are mostly
// byte-identical to the pre-move versions in DataArea, except:
//   • field names use the typed sub-structs (health.* instead of m_health_*)
//   • MQ API calls go through m_api->XMQ* (PR D infrastructure)
//   • trace logging stays in the DataArea wrappers — no callback needed
//   • setErrorMsg also stays in the DataArea wrappers (cc/rc returned)
// ─────────────────────────────────────────────────────────────────────────

bool MQConnection::startHealthMonitor(MQLONG& cc, MQLONG& rc)
{
    cc = MQCC_OK;
    rc = MQRC_NONE;

    // Guard: nothing to do if not connected, already active, or no api.
    if (!m_connected || health.check_active || m_api == NULL)
        return false;

    // Open the QM object for inquire — used as a lightweight health probe.
    MQOD od = {MQOD_DEFAULT};
    od.ObjectType = MQOT_Q_MGR;
    m_api->XMQOpen(m_qm, &od, MQOO_INQUIRE | MQOO_FAIL_IF_QUIESCING,
                   &health.check_handle, &cc, &rc);

    if (cc == MQCC_OK)
    {
        health.check_active          = TRUE;
        health.status                = 1;  // healthy
        DWORD now                    = GetTickCount();
        health.connection_start_time = now;
        health.last_check_time       = now;
        health.check_count           = 0;
        health.check_failures        = 0;
        return true;
    }

    // Open failed — monitor won't run but connection is still usable.
    health.check_handle = MQHO_NONE;
    health.check_active = FALSE;
    return false;
}

void MQConnection::stopHealthMonitor(MQLONG& cc, MQLONG& rc)
{
    cc = MQCC_OK;
    rc = MQRC_NONE;

    if (health.check_handle != MQHO_NONE && m_connected && m_api != NULL)
    {
        m_api->XMQClose(m_qm, &health.check_handle, MQCO_NONE, &cc, &rc);
    }

    health.check_handle = MQHO_NONE;
    health.check_active = FALSE;
    health.status       = 0;  // disconnected
}

void MQConnection::disconnect(MQLONG& cc, MQLONG& rc)
{
    cc = MQCC_OK;
    rc = MQRC_NONE;

    // Best-effort stop of the health monitor before tearing down the
    // connection — its cached handle lives off this MQHCONN.
    MQLONG dummyCc, dummyRc;
    stopHealthMonitor(dummyCc, dummyRc);

    if (m_qm != MQHO_NONE && m_api != NULL && m_connected)
    {
        m_api->XMQDisc(&m_qm, &cc, &rc);
    }

    m_connected = false;
    m_qm        = MQHO_NONE;
    current.qm_name.Empty();
}

void MQConnection::notifyConnectionLost()
{
    // Connection identity gone.
    m_connected = false;
    m_qm        = MQHO_NONE;
    current.qm_name.Empty();
    current.userid.Empty();

    // Mark involuntary loss so the next successful connect (via
    // checkConnection or attemptReconnection) emits a "Reconnected" line
    // in the message log.
    reconnect.connection_was_lost = TRUE;

    // Health monitor state — handle is now invalid.
    health.check_handle = MQHO_NONE;
    health.check_active = FALSE;
    health.status       = 0;
}

MQConnection::~MQConnection()
{
    // RAII: if still connected at destruction (e.g., app shutdown that
    // skipped explicitDiscQM), make a best-effort XMQDisc call so the
    // handle doesn't leak. Errors are swallowed — no one to surface them
    // to at this point.
    if (m_qm != MQHO_NONE && m_connected && m_api != NULL)
    {
        MQLONG cc = MQCC_OK;
        MQLONG rc = MQRC_NONE;
        m_api->XMQDisc(&m_qm, &cc, &rc);
    }
}

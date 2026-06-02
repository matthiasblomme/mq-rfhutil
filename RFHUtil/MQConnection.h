#pragma once

#include "cmqc.h"

// P4.1 — passive holder for the queue-manager connection.
//
// PR A scope: this class only owns the MQHCONN handle and the connected
// flag. Settings (TLS, credentials, heartbeat, reconnect, health monitor)
// move here in PR B. Lifecycle and recovery methods move in PR C. Call
// sites get migrated in PR D, at which point DataArea's reference aliases
// (`qm`, `connected`) go away.
//
// During PRs A-C, DataArea exposes `qm` and `connected` as references that
// point into this struct, so existing code in DataArea.cpp / General.cpp /
// rfhutilView.cpp / ConnSettings.cpp keeps compiling unchanged.
class MQConnection
{
public:
    MQConnection() : m_qm(MQHO_NONE), m_connected(false) {}

    MQHCONN m_qm;
    bool    m_connected;
};

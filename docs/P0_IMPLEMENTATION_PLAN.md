# P0 Implementation Plan - Connection Reliability Improvements

## Overview

This document provides **step-by-step implementation instructions** for the three P0 (Priority 0) improvements to RFHUtil's connection reliability.

---

## Three P0 Improvements

### P0.1: Add HeartBeat/KeepAlive Configuration
**Effort:** 4 hours | **Impact:** High | **Risk:** Low

### P0.2: Implement Automatic Reconnection
**Effort:** 18 hours | **Impact:** High | **Risk:** Low

### P0.3: Add UI Tab for Connection Settings
**Effort:** 12 hours | **Impact:** High | **Risk:** Low

**Total Effort:** 34 hours (4-5 days)

---

## P0.1: Add HeartBeat/KeepAlive Configuration

### Goal
Add explicit HeartBeat and KeepAlive configuration to the connection setup for faster failure detection and firewall timeout prevention.

### Files to Modify
1. `RFHUtil/DataArea.cpp` - Add configuration code
2. `RFHUtil/DataArea.h` - Add member variables (for UI control)

### Implementation Steps

#### Step 1: Add Member Variables to DataArea.h

**File:** `RFHUtil/DataArea.h`

**Location:** Find the class member variables section (around line 200-300)

**Add these variables:**

```cpp
// Connection reliability settings
int m_heartbeat_interval;      // HeartBeat interval in seconds (0=disabled, default=60)
int m_keepalive_interval;      // KeepAlive interval in seconds (-1=AUTO, 0=disabled)
bool m_enable_auto_reconnect;  // Enable automatic reconnection
int m_reconnect_max_attempts;  // Maximum reconnection attempts (default=3)
int m_reconnect_delay_ms;      // Initial reconnection delay in ms (default=2000)
```

#### Step 2: Initialize Variables in Constructor

**File:** `RFHUtil/DataArea.cpp`

**Location:** In `DataArea::DataArea()` constructor (around line 123-290)

**Add initialization:**

```cpp
// Initialize connection reliability settings with defaults
m_heartbeat_interval = 60;           // 60 seconds
m_keepalive_interval = -1;           // MQKAI_AUTO
m_enable_auto_reconnect = true;      // Enabled by default
m_reconnect_max_attempts = 3;        // 3 attempts
m_reconnect_delay_ms = 2000;         // 2 seconds initial delay
```

#### Step 3: Add Configuration Code to connect2QM

**File:** `RFHUtil/DataArea.cpp`

**Location:** In `DataArea::connect2QM()` method, after line 10407 (after `cd.MaxMsgLength = 104857600;`)

**Add this code:**

```cpp
// ============================================================
// P0.1: Add HeartBeat and KeepAlive configuration
// ============================================================

// Set HeartBeat interval for fast MQ-level failure detection
// This detects queue manager crashes, process failures, and keeps
// the connection active to prevent firewall timeouts
if (m_heartbeat_interval > 0) {
    cd.HeartBeatInterval = m_heartbeat_interval;
    
    if (traceEnabled) {
        sprintf(traceInfo, "Setting HeartBeatInterval=%d seconds", m_heartbeat_interval);
        logTraceEntry(traceInfo);
    }
}

// Set KeepAlive interval for network-level failure detection
// This detects network cable unplugs, router failures, and
// "half-open" TCP connections
if (m_keepalive_interval == -1) {
    // Use OS defaults (MQKAI_AUTO)
    cd.KeepAliveInterval = MQKAI_AUTO;
    
    if (traceEnabled) {
        logTraceEntry("Setting KeepAliveInterval=AUTO (OS default)");
    }
} else if (m_keepalive_interval > 0) {
    // Use specified interval
    cd.KeepAliveInterval = m_keepalive_interval;
    
    if (traceEnabled) {
        sprintf(traceInfo, "Setting KeepAliveInterval=%d seconds", m_keepalive_interval);
        logTraceEntry(traceInfo);
    }
}
// If m_keepalive_interval == 0, KeepAlive is disabled (not recommended)

// Ensure we're using a version that supports these fields
if (cd.Version < MQCD_VERSION_7) {
    cd.Version = MQCD_VERSION_7;
    cd.StrucLength = MQCD_LENGTH_7;
    
    if (traceEnabled) {
        logTraceEntry("Upgraded MQCD to version 7 for HeartBeat/KeepAlive support");
    }
}
```

#### Step 4: Test the Implementation

**Test Procedure:**

1. **Build the project:**
   ```cmd
   msbuild RFHUtil.sln /p:Configuration=Release /p:Platform=Win32
   ```

2. **Enable trace:**
   ```cmd
   set RFHUTIL_TRACE_FILE=c:\temp\rfhutil_trace.log
   ```

3. **Run RFHUtil and connect to QM1:**
   - Start rfhutilc.exe
   - Connect to QM1
   - Check trace log for HeartBeat/KeepAlive settings

4. **Verify in trace log:**
   ```
   Setting HeartBeatInterval=60 seconds
   Setting KeepAliveInterval=AUTO (OS default)
   Upgraded MQCD to version 7 for HeartBeat/KeepAlive support
   ```

5. **Test idle connection:**
   - Leave connection idle for 5+ minutes
   - Verify connection stays alive
   - Try to browse queue after idle period

**Expected Results:**
- ✅ HeartBeat set to 60 seconds
- ✅ KeepAlive set to AUTO
- ✅ Connection stays alive during idle periods
- ✅ No firewall timeouts

---

## P0.2: Implement Automatic Reconnection

### Goal
Add automatic reconnection with exponential backoff when connection is lost.

### Files to Create/Modify
1. `RFHUtil/ConnectionManager.h` - New file
2. `RFHUtil/ConnectionManager.cpp` - New file
3. `RFHUtil/DataArea.h` - Add ConnectionManager member
4. `RFHUtil/DataArea.cpp` - Integrate ConnectionManager
5. `RFHUtil/RFHUtil.vcxproj` - Add new files to project

### Implementation Steps

#### Step 1: Create ConnectionManager.h

**File:** `RFHUtil/ConnectionManager.h` (NEW FILE)

```cpp
/*
 * ConnectionManager.h
 * Manages MQ connections with automatic reconnection
 * 
 * Copyright (c) IBM Corporation 2024
 * Licensed under the Apache License, Version 2.0
 */

#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include "cmqc.h"
#include <string>

class ConnectionManager {
public:
    // Configuration structure
    struct Config {
        int maxRetries = 3;              // Maximum reconnection attempts
        int initialDelayMs = 2000;       // Initial delay (2 seconds)
        int maxDelayMs = 30000;          // Maximum delay (30 seconds)
        double backoffMultiplier = 2.0;  // Exponential backoff multiplier
        bool enableAutoReconnect = true; // Enable automatic reconnection
    };

    // Connection state
    enum State {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        RECONNECTING,
        FAILED
    };

    ConnectionManager();
    ~ConnectionManager();

    // Connection management
    bool connect(const char* qmName, const Config& config = Config());
    bool disconnect();
    bool isConnected() const;
    bool ensureConnected();  // Check and reconnect if needed
    State getState() const { return state; }

    // Health monitoring
    bool isHealthy();
    MQLONG getLastCompletionCode() const { return lastCC; }
    MQLONG getLastReasonCode() const { return lastRC; }
    const char* getLastError() const { return lastError.c_str(); }

    // Connection handle
    MQHCONN getHandle() const { return qm; }

    // Configuration
    void setConfig(const Config& cfg) { config = cfg; }
    Config getConfig() const { return config; }

private:
    MQHCONN qm;
    State state;
    std::string currentQM;
    Config config;
    
    // Error tracking
    MQLONG lastCC;
    MQLONG lastRC;
    std::string lastError;
    
    // Reconnection state
    int reconnectAttempts;
    DWORD lastReconnectTime;

    // Internal methods
    bool connectInternal(const char* qmName);
    bool reconnect();
    int calculateBackoffDelay(int attempt);
    void logConnectionEvent(const char* event);
    void setState(State newState);
};

#endif // CONNECTION_MANAGER_H
```

#### Step 2: Create ConnectionManager.cpp

**File:** `RFHUtil/ConnectionManager.cpp` (NEW FILE)

```cpp
/*
 * ConnectionManager.cpp
 * Implementation of automatic reconnection logic
 * 
 * Copyright (c) IBM Corporation 2024
 * Licensed under the Apache License, Version 2.0
 */

#include "stdafx.h"
#include "ConnectionManager.h"
#include "comsubs.h"
#include <algorithm>
#include <cmath>

// External trace flag (from DataArea)
extern BOOL traceEnabled;
extern void logTraceEntry(const char* msg);

ConnectionManager::ConnectionManager()
    : qm(MQHO_NONE)
    , state(DISCONNECTED)
    , lastCC(MQCC_OK)
    , lastRC(MQRC_NONE)
    , reconnectAttempts(0)
    , lastReconnectTime(0)
{
    logConnectionEvent("ConnectionManager created");
}

ConnectionManager::~ConnectionManager()
{
    if (state == CONNECTED) {
        disconnect();
    }
    logConnectionEvent("ConnectionManager destroyed");
}

bool ConnectionManager::connect(const char* qmName, const Config& cfg)
{
    config = cfg;
    currentQM = qmName;
    
    setState(CONNECTING);
    
    if (!config.enableAutoReconnect) {
        // Simple connection without retry
        bool result = connectInternal(qmName);
        setState(result ? CONNECTED : FAILED);
        return result;
    }
    
    // Try connection with retries
    for (int attempt = 0; attempt < config.maxRetries; attempt++) {
        reconnectAttempts = attempt;
        
        char msg[256];
        sprintf(msg, "Connection attempt %d/%d to QM: %s", 
                attempt + 1, config.maxRetries, qmName);
        logConnectionEvent(msg);
        
        if (connectInternal(qmName)) {
            reconnectAttempts = 0;
            setState(CONNECTED);
            logConnectionEvent("Connection successful");
            return true;
        }
        
        // If not last attempt, wait before retry
        if (attempt < config.maxRetries - 1) {
            int delay = calculateBackoffDelay(attempt);
            
            sprintf(msg, "Connection failed (attempt %d/%d), retrying in %dms", 
                    attempt + 1, config.maxRetries, delay);
            logConnectionEvent(msg);
            
            Sleep(delay);
        }
    }
    
    setState(FAILED);
    logConnectionEvent("Connection failed after all retries");
    return false;
}

bool ConnectionManager::connectInternal(const char* qmName)
{
    // NOTE: This is a simplified version
    // In the actual implementation, this would call the existing
    // DataArea::connect2QM() logic with all the proper setup
    
    // For now, this shows the structure
    // The actual implementation would need to:
    // 1. Set up MQCNO, MQCD, MQCSP structures
    // 2. Configure HeartBeat/KeepAlive
    // 3. Handle SSL/TLS if needed
    // 4. Call MQCONNX
    
    // Placeholder - actual implementation in DataArea integration
    lastCC = MQCC_FAILED;
    lastRC = MQRC_NOT_CONNECTED;
    lastError = "Not implemented - use DataArea::connect2QM()";
    
    return false;
}

bool ConnectionManager::disconnect()
{
    if (state != CONNECTED || qm == MQHO_NONE) {
        setState(DISCONNECTED);
        return true;
    }
    
    logConnectionEvent("Disconnecting from queue manager");
    
    MQDISC(&qm, &lastCC, &lastRC);
    
    setState(DISCONNECTED);
    qm = MQHO_NONE;
    
    if (lastCC == MQCC_OK) {
        logConnectionEvent("Disconnected successfully");
        return true;
    } else {
        char msg[256];
        sprintf(msg, "Disconnect failed: CC=%d, RC=%d", lastCC, lastRC);
        logConnectionEvent(msg);
        return false;
    }
}

bool ConnectionManager::isConnected() const
{
    return (state == CONNECTED && qm != MQHO_NONE);
}

bool ConnectionManager::ensureConnected()
{
    if (!isConnected()) {
        logConnectionEvent("Not connected - cannot ensure connection");
        return false;
    }
    
    // Perform lightweight health check
    if (!isHealthy()) {
        logConnectionEvent("Connection unhealthy - attempting reconnect");
        return reconnect();
    }
    
    return true;
}

bool ConnectionManager::isHealthy()
{
    if (state != CONNECTED) {
        return false;
    }
    
    // Perform a lightweight MQINQ to check connection health
    MQLONG selector = MQIA_Q_MGR_STATUS;
    MQLONG value;
    
    MQINQ(qm, MQOT_Q_MGR, NULL, 1, &selector, 0, NULL, 
          sizeof(value), &value, &lastCC, &lastRC);
    
    bool healthy = (lastCC == MQCC_OK);
    
    if (!healthy) {
        char msg[256];
        sprintf(msg, "Health check failed: CC=%d, RC=%d", lastCC, lastRC);
        logConnectionEvent(msg);
    }
    
    return healthy;
}

bool ConnectionManager::reconnect()
{
    logConnectionEvent("Starting reconnection process");
    
    setState(RECONNECTING);
    
    // Disconnect if still connected
    if (qm != MQHO_NONE) {
        disconnect();
    }
    
    // Attempt reconnection
    bool result = connect(currentQM.c_str(), config);
    
    if (result) {
        logConnectionEvent("Reconnection successful");
    } else {
        logConnectionEvent("Reconnection failed");
    }
    
    return result;
}

int ConnectionManager::calculateBackoffDelay(int attempt)
{
    // Exponential backoff: delay = initial * (multiplier ^ attempt)
    double delay = config.initialDelayMs * 
                   pow(config.backoffMultiplier, (double)attempt);
    
    // Cap at maximum delay
    int finalDelay = (int)std::min(delay, (double)config.maxDelayMs);
    
    return finalDelay;
}

void ConnectionManager::logConnectionEvent(const char* event)
{
    // Log to trace if enabled
    if (traceEnabled) {
        char msg[512];
        sprintf(msg, "ConnectionManager: %s (QM=%s, State=%d, Attempt=%d)", 
                event, currentQM.c_str(), state, reconnectAttempts);
        logTraceEntry(msg);
    }
}

void ConnectionManager::setState(State newState)
{
    if (state != newState) {
        State oldState = state;
        state = newState;
        
        char msg[256];
        sprintf(msg, "State changed: %d -> %d", oldState, newState);
        logConnectionEvent(msg);
    }
}
```

#### Step 3: Integrate ConnectionManager with DataArea

**File:** `RFHUtil/DataArea.h`

**Add to class members:**

```cpp
#include "ConnectionManager.h"

class DataArea {
    // ... existing members ...
    
private:
    ConnectionManager* connMgr;  // Connection manager for auto-reconnect
    
public:
    // Add new methods
    bool connectWithRetry(const char* qmName);
    bool ensureConnection();
    ConnectionManager* getConnectionManager() { return connMgr; }
};
```

**File:** `RFHUtil/DataArea.cpp`

**In constructor:**

```cpp
DataArea::DataArea()
{
    // ... existing initialization ...
    
    // Initialize connection manager
    connMgr = new ConnectionManager();
    
    if (traceEnabled) {
        logTraceEntry("DataArea: ConnectionManager initialized");
    }
}
```

**In destructor:**

```cpp
DataArea::~DataArea()
{
    // ... existing cleanup ...
    
    if (connMgr) {
        delete connMgr;
        connMgr = NULL;
    }
}
```

**Add new methods:**

```cpp
// Connect with automatic retry
bool DataArea::connectWithRetry(const char* qmName)
{
    if (traceEnabled) {
        char msg[256];
        sprintf(msg, "DataArea::connectWithRetry() QM=%s", qmName);
        logTraceEntry(msg);
    }
    
    // Configure connection manager
    ConnectionManager::Config config;
    config.maxRetries = m_reconnect_max_attempts;
    config.initialDelayMs = m_reconnect_delay_ms;
    config.maxDelayMs = 30000;
    config.backoffMultiplier = 2.0;
    config.enableAutoReconnect = m_enable_auto_reconnect;
    
    connMgr->setConfig(config);
    
    // For now, use existing connect2QM
    // In full implementation, ConnectionManager would handle the connection
    bool result = connect2QM(qmName);
    
    if (!result && m_enable_auto_reconnect) {
        // Retry with backoff
        for (int attempt = 1; attempt < m_reconnect_max_attempts; attempt++) {
            int delay = m_reconnect_delay_ms * (int)pow(2.0, (double)(attempt - 1));
            if (delay > 30000) delay = 30000;
            
            if (traceEnabled) {
                char msg[256];
                sprintf(msg, "Retry attempt %d after %dms", attempt + 1, delay);
                logTraceEntry(msg);
            }
            
            Sleep(delay);
            
            result = connect2QM(qmName);
            if (result) break;
        }
    }
    
    return result;
}

// Ensure connection is healthy
bool DataArea::ensureConnection()
{
    if (!connected) {
        return false;
    }
    
    // Check if connection is still valid
    MQLONG selector = MQIA_Q_MGR_STATUS;
    MQLONG value;
    MQLONG cc, rc;
    
    MQINQ(qm, MQOT_Q_MGR, NULL, 1, &selector, 0, NULL, 
          sizeof(value), &value, &cc, &rc);
    
    if (cc != MQCC_OK) {
        if (traceEnabled) {
            char msg[256];
            sprintf(msg, "Connection health check failed: CC=%d, RC=%d", cc, rc);
            logTraceEntry(msg);
        }
        
        // Attempt reconnection if enabled
        if (m_enable_auto_reconnect) {
            return connectWithRetry((LPCTSTR)currentQM);
        }
        
        return false;
    }
    
    return true;
}
```

#### Step 4: Add Files to Visual Studio Project

**File:** `RFHUtil/RFHUtil.vcxproj`

**Add to `<ItemGroup>` with other `.cpp` files:**

```xml
<ClCompile Include="ConnectionManager.cpp" />
```

**Add to `<ItemGroup>` with other `.h` files:**

```xml
<ClInclude Include="ConnectionManager.h" />
```

#### Step 5: Test Automatic Reconnection

**Test Procedure:**

1. **Build the project**

2. **Test normal connection:**
   ```
   - Connect to QM1
   - Verify connection successful
   ```

3. **Test reconnection:**
   ```
   - Connect to QM1
   - docker stop qm1
   - Wait for detection (~60s)
   - docker start qm1
   - Verify automatic reconnection
   ```

4. **Test retry logic:**
   ```
   - Ensure QM1 is stopped
   - Try to connect
   - Verify 3 retry attempts with backoff (2s, 4s, 8s)
   - Start QM1 during retry
   - Verify successful connection
   ```

**Expected Results:**
- ✅ Automatic reconnection after QM restart
- ✅ Exponential backoff (2s, 4s, 8s)
- ✅ Maximum 3 retry attempts
- ✅ Transparent to user (no manual intervention)

---

## P0.3: Add UI Tab for Connection Settings

### Goal
Add a new tab in the property sheet to allow users to configure HeartBeat, KeepAlive, and automatic reconnection settings.

### Files to Create/Modify
1. `RFHUtil/ConnSettings.h` - New file (dialog header)
2. `RFHUtil/ConnSettings.cpp` - New file (dialog implementation)
3. `RFHUtil/resource.h` - Add dialog resource IDs
4. `RFHUtil/rfhutil.rc` - Add dialog resource
5. `RFHUtil/MyPropertySheet.cpp` - Add new tab
6. `RFHUtil/RFHUtil.vcxproj` - Add new files

### Implementation Steps

#### Step 1: Create ConnSettings.h

**File:** `RFHUtil/ConnSettings.h` (NEW FILE)

```cpp
/*
 * ConnSettings.h
 * Connection Settings property page
 * 
 * Copyright (c) IBM Corporation 2024
 * Licensed under the Apache License, Version 2.0
 */

#if !defined(AFX_CONNSETTINGS_H__INCLUDED_)
#define AFX_CONNSETTINGS_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// ConnSettings dialog

class ConnSettings : public CPropertyPage
{
    DECLARE_DYNCREATE(ConnSettings)

// Construction
public:
    ConnSettings();
    ~ConnSettings();
    
    DataArea* pDoc;

// Dialog Data
    //{{AFX_DATA(ConnSettings)
    enum { IDD = IDD_CONN_SETTINGS };
    int     m_heartbeat_interval;
    int     m_keepalive_interval;
    BOOL    m_enable_auto_reconnect;
    int     m_reconnect_max_attempts;
    int     m_reconnect_delay_ms;
    CString m_status_text;
    //}}AFX_DATA

// Overrides
    //{{AFX_VIRTUAL(ConnSettings)
    public:
    virtual BOOL OnSetActive();
    virtual BOOL OnKillActive();
    virtual void OnOK();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    //}}AFX_VIRTUAL

// Implementation
protected:
    //{{AFX_MSG(ConnSettings)
    virtual BOOL OnInitDialog();
    afx_msg void OnEnableAutoReconnect();
    afx_msg void OnResetDefaults();
    afx_msg void OnApplySettings();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    void UpdateControls();
    void LoadSettings();
    void SaveSettings();
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CONNSETTINGS_H__INCLUDED_)
```

#### Step 2: Create ConnSettings.cpp

**File:** `RFHUtil/ConnSettings.cpp` (NEW FILE)

```cpp
/*
 * ConnSettings.cpp
 * Connection Settings property page implementation
 * 
 * Copyright (c) IBM Corporation 2024
 * Licensed under the Apache License, Version 2.0
 */

#include "stdafx.h"
#include "rfhutil.h"
#include "ConnSettings.h"
#include "DataArea.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ConnSettings property page

IMPLEMENT_DYNCREATE(ConnSettings, CPropertyPage)

ConnSettings::ConnSettings() : CPropertyPage(ConnSettings::IDD)
{
    //{{AFX_DATA_INIT(ConnSettings)
    m_heartbeat_interval = 60;
    m_keepalive_interval = -1;  // AUTO
    m_enable_auto_reconnect = TRUE;
    m_reconnect_max_attempts = 3;
    m_reconnect_delay_ms = 2000;
    m_status_text = _T("");
    //}}AFX_DATA_INIT
    
    pDoc = NULL;
}

ConnSettings::~ConnSettings()
{
}

void ConnSettings::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(ConnSettings)
    DDX_Text(pDX, IDC_HEARTBEAT_INTERVAL, m_heartbeat_interval);
    DDV_MinMaxInt(pDX, m_heartbeat_interval, 0, 999999);
    DDX_Text(pDX, IDC_KEEPALIVE_INTERVAL, m_keepalive_interval);
    DDV_MinMaxInt(pDX, m_keepalive_interval, -1, 999999);
    DDX_Check(pDX, IDC_ENABLE_AUTO_RECONNECT, m_enable_auto_reconnect);
    DDX_Text(pDX, IDC_RECONNECT_MAX_ATTEMPTS, m_reconnect_max_attempts);
    DDV_MinMaxInt(pDX, m_reconnect_max_attempts, 1, 10);
    DDX_Text(pDX, IDC_RECONNECT_DELAY_MS, m_reconnect_delay_ms);
    DDV_MinMaxInt(pDX, m_reconnect_delay_ms, 100, 60000);
    DDX_Text(pDX, IDC_CONN_STATUS_TEXT, m_status_text);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(ConnSettings, CPropertyPage)
    //{{AFX_MSG_MAP(ConnSettings)
    ON_BN_CLICKED(IDC_ENABLE_AUTO_RECONNECT, OnEnableAutoReconnect)
    ON_BN_CLICKED(IDC_RESET_DEFAULTS, OnResetDefaults)
    ON_BN_CLICKED(IDC_APPLY_SETTINGS, OnApplySettings)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ConnSettings message handlers

BOOL ConnSettings::OnInitDialog()
{
    CPropertyPage::OnInitDialog();
    
    // Load current settings
    LoadSettings();
    
    // Update control states
    UpdateControls();
    
    return TRUE;
}

BOOL ConnSettings::OnSetActive()
{
    // Reload settings when tab becomes active
    LoadSettings();
    UpdateData(FALSE);
    
    return CPropertyPage::OnSetActive();
}

BOOL ConnSettings::OnKillActive()
{
    if (!UpdateData(TRUE))
        return FALSE;
    
    // Save settings when leaving tab
    SaveSettings();
    
    return CPropertyPage::OnKillActive();
}

void ConnSettings::OnOK()
{
    UpdateData(TRUE);
    SaveSettings();
    CPropertyPage::OnOK();
}

void ConnSettings::OnEnableAutoReconnect()
{
    UpdateData(TRUE);
    UpdateControls();
}

void ConnSettings::OnResetDefaults()
{
    m_heartbeat_interval = 60;
    m_keepalive_interval = -1;  // AUTO
    m_enable_auto_reconnect = TRUE;
    m_reconnect_max_attempts = 3;
    m_reconnect_delay_ms = 2000;
    
    UpdateData(FALSE);
    UpdateControls();
    
    m_status_text = _T("Settings reset to defaults");
    UpdateData(FALSE);
}

void ConnSettings::OnApplySettings()
{
    UpdateData(TRUE);
    SaveSettings();
    
    m_status_text = _T("Settings applied successfully");
    UpdateData(FALSE);
}

void ConnSettings::UpdateControls()
{
    // Enable/disable reconnection settings based on checkbox
    GetDlgItem(IDC_RECONNECT_MAX_ATTEMPTS)->EnableWindow(m_enable_auto_reconnect);
    GetDlgItem(IDC_RECONNECT_DELAY_MS)->EnableWindow(m_enable_auto_reconnect);
}

void ConnSettings::LoadSettings()
{
    if (pDoc == NULL)
        return;
    
    // Load settings from DataArea
    m_heartbeat_interval = pDoc->m_heartbeat_interval;
    m_keepalive_interval = pDoc->m_keepalive_interval;
    m_enable_auto_reconnect = pDoc->m_enable_auto_reconnect;
    m_reconnect_max_attempts = pDoc->m_reconnect_max_attempts;
    m_reconnect_delay_ms = pDoc->m_reconnect_delay_ms;
    
    // Update status
    if (pDoc->connected) {
        m_status_text = _T("Connected - Settings will apply to next connection");
    } else {
        m_status_text = _T("Not connected - Settings will apply when connecting");
    }
}

void ConnSettings::SaveSettings()
{
    if (pDoc == NULL)
        return;
    
    // Save settings to DataArea
    pDoc->m_heartbeat_interval = m_heartbeat_interval;
    pDoc->m_keepalive_interval = m_keepalive_interval;
    pDoc->m_enable_auto_reconnect = m_enable_auto_reconnect;
    pDoc->m_reconnect_max_attempts = m_reconnect_max_attempts;
    pDoc->m_reconnect_delay_ms = m_reconnect_delay_ms;
}
```

#### Step 3: Add Dialog Resource

**File:** `RFHUtil/resource.h`

**Add these defines:**

```cpp
#define IDD_CONN_SETTINGS               2000
#define IDC_HEARTBEAT_INTERVAL          2001
#define IDC_KEEPALIVE_INTERVAL          2002
#define IDC_ENABLE_AUTO_RECONNECT       2003
#define IDC_RECONNECT_MAX_ATTEMPTS      2004
#define IDC_RECONNECT_DELAY_MS          2005
#define IDC_RESET_DEFAULTS              2006
#define IDC_APPLY_SETTINGS              2007
#define IDC_CONN_STATUS_TEXT            2008
```

**File:** `RFHUtil/rfhutil.rc`

**Add dialog resource (you'll need to add this in the Visual Studio resource editor or manually):**

```rc
IDD_CONN_SETTINGS DIALOGEX 0, 0, 320, 240
STYLE DS_SETFONT | DS_FIXEDSYS | WS_CHILD | WS_DISABLED | WS_CAPTION
CAPTION "Connection"
FONT 8, "MS Shell Dlg", 0, 0, 0x1
BEGIN
    GROUPBOX        "HeartBeat Settings",IDC_STATIC,7,7,306,60
    LTEXT           "HeartBeat Interval (seconds):",IDC_STATIC,15,22,100,8
    EDITTEXT        IDC_HEARTBEAT_INTERVAL,120,20,50,14,ES_AUTOHSCROLL | ES_NUMBER
    LTEXT           "0 = Disabled, Recommended: 60",IDC_STATIC,175,22,130,8
    LTEXT           "HeartBeat detects MQ failures and prevents firewall timeouts.",IDC_STATIC,15,40,280,8
    LTEXT           "Lower values = faster detection, but more network traffic.",IDC_STATIC,15,52,280,8
    
    GROUPBOX        "KeepAlive Settings",IDC_STATIC,7,72,306,60
    LTEXT           "KeepAlive Interval (seconds):",IDC_STATIC,15,87,100,8
    EDITTEXT        IDC_KEEPALIVE_INTERVAL,120,85,50,14,ES_AUTOHSCROLL | ES_NUMBER
    LTEXT           "-1 = AUTO (OS default), 0 = Disabled",IDC_STATIC,175,87,130,8
    LTEXT           "KeepAlive detects network failures at TCP level.",IDC_STATIC,15,105,280,8
    LTEXT           "AUTO is recommended for most environments.",IDC_STATIC,15,117,280,8
    
    GROUPBOX        "Automatic Reconnection",IDC_STATIC,7,137,306,70
    CONTROL         "Enable Automatic Reconnection",IDC_ENABLE_AUTO_RECONNECT,
                    "Button",BS_AUTOCHECKBOX | WS_TABSTOP,15,152,150,10
    LTEXT           "Maximum Retry Attempts:",IDC_STATIC,15,167,100,8
    EDITTEXT        IDC_RECONNECT_MAX_ATTEMPTS,120,165,50,14,ES_AUTOHSCROLL | ES_NUMBER
    LTEXT           "Initial Delay (milliseconds):",IDC_STATIC,15,187,100,8
    EDITTEXT        IDC_RECONNECT_DELAY_MS,120,185,50,14,ES_AUTOHSCROLL | ES_NUMBER
    LTEXT           "Uses exponential backoff (2x each attempt)",IDC_STATIC,175,187,130,8
    
    PUSHBUTTON      "Reset to Defaults",IDC_RESET_DEFAULTS,7,215,70,18
    PUSHBUTTON      "Apply Settings",IDC_APPLY_SETTINGS,82,215,70,18
    LTEXT           "",IDC_CONN_STATUS_TEXT,160,218,150,12
END
```

#### Step 4: Add Tab to Property Sheet

**File:** `RFHUtil/MyPropertySheet.cpp`

**Add include:**

```cpp
#include "ConnSettings.h"
```

**In constructor, add the new page:**

```cpp
CMyPropertySheet::CMyPropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
    : CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
    // ... existing pages ...
    
    // Add Connection Settings page
    AddPage(&m_connSettings);
}
```

**In header file (`MyPropertySheet.h`), add member:**

```cpp
#include "ConnSettings.h"

class CMyPropertySheet : public CPropertySheet
{
    // ... existing members ...
    
    ConnSettings m_connSettings;
};
```

#### Step 5: Add Files to Project

**File:** `RFHUtil/RFHUtil.vcxproj`

**Add to `<ItemGroup>` sections:**

```xml
<ClCompile Include="ConnSettings.cpp" />
<ClInclude Include="ConnSettings.h" />
```

#### Step 6: Test the UI

**Test Procedure:**

1. **Build and run RFHUtil**

2. **Open Connection Settings tab:**
   - Should see new "Connection" tab
   - Default values should be loaded

3. **Test controls:**
   - Change HeartBeat interval to 30
   - Change KeepAlive to 60
   - Enable/disable auto-reconnect
   - Verify controls enable/disable correctly

4. **Test Apply:**
   - Click "Apply Settings"
   - Verify status message
   - Connect to QM1
   - Check trace log for new settings

5. **Test Reset:**
   - Change all values
   - Click "Reset to Defaults"
   - Verify values return to defaults

**Expected Results:**
- ✅ New "Connection" tab visible
- ✅ All controls working
- ✅ Settings persist across connections
- ✅ Apply and Reset buttons working
- ✅ Status messages displayed

---

## Testing Checklist

### P0.1: HeartBeat/KeepAlive
- [ ] Build succeeds
- [ ] Trace shows HeartBeat=60
- [ ] Trace shows KeepAlive=AUTO
- [ ] Connection stays alive 10+ minutes
- [ ] No firewall timeouts

### P0.2: Auto-Reconnection
- [ ] Build succeeds
- [ ] Normal connection works
- [ ] QM restart triggers reconnection
- [ ] Retry attempts logged (3 attempts)
- [ ] Exponential backoff working (2s, 4s, 8s)
- [ ] Successful reconnection after QM restart

### P0.3: UI Tab
- [ ] Build succeeds
- [ ] New tab visible
- [ ] All controls present
- [ ] Default values correct
- [ ] Apply button works
- [ ] Reset button works
- [ ] Settings persist
- [ ] Status messages displayed

---

## Rollback Plan

If issues occur:

1. **Keep backup of original files:**
   ```cmd
   copy RFHUtil\DataArea.cpp RFHUtil\DataArea.cpp.backup
   copy RFHUtil\DataArea.h RFHUtil\DataArea.h.backup
   ```

2. **Git branch strategy:**
   ```bash
   git checkout -b feature/p0-connection-improvements
   # Make changes
   git commit -m "P0: Add connection improvements"
   # If issues:
   git checkout main
   ```

3. **Incremental deployment:**
   - Deploy P0.1 first (lowest risk)
   - Test thoroughly
   - Deploy P0.2
   - Test thoroughly
   - Deploy P0.3

---

## Success Criteria

**P0.1 Success:**
- ✅ HeartBeat configured to 60s
- ✅ KeepAlive configured to AUTO
- ✅ No build errors
- ✅ Connection stable for 10+ minutes
- ✅ Trace logs show configuration

**P0.2 Success:**
- ✅ Automatic reconnection working
- ✅ 3 retry attempts with backoff
- ✅ Successful reconnection after QM restart
- ✅ No manual intervention needed
- ✅ Trace logs show reconnection attempts

**P0.3 Success:**
- ✅ New UI tab visible and functional
- ✅ All controls working correctly
- ✅ Settings persist across sessions
- ✅ Apply/Reset buttons working
- ✅ User-friendly interface

---

## Timeline

**Day 1 (4 hours):**
- Morning: Implement P0.1 (HeartBeat/KeepAlive)
- Afternoon: Test P0.1 with QM1

**Day 2-3 (16 hours):**
- Implement P0.2 (Auto-reconnection)
- Create ConnectionManager class
- Integrate with DataArea
- Test reconnection scenarios

**Day 4-5 (12 hours):**
- Implement P0.3 (UI Tab)
- Create ConnSettings dialog
- Add to property sheet
- Test UI functionality

**Day 5 (2 hours):**
- Final integration testing
- Documentation updates
- Prepare for deployment

**Total: 34 hours (4-5 days)**

---

**Document Version:** 1.0  
**Date:** 2026-02-13  
**Author:** IBM Bob (Architect Mode)  
**Status:** Ready for Implementation
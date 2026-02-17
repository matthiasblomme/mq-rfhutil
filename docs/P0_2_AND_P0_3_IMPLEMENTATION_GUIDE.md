# P0.2 and P0.3 Implementation Guide

## Overview
This document provides detailed implementation instructions for:
- **P0.2**: Automatic Reconnection (18 hours estimated)
- **P0.3**: UI Tab for Connection Settings (12 hours estimated)

## Current Status
✅ **P0.1 COMPLETED**: HeartBeat/KeepAlive configuration
- HeartbeatInterval: 60 seconds
- KeepAliveInterval: MQKAI_AUTO
- MQCD Version: 6

---

## P0.2: Automatic Reconnection Implementation

### Objective
Automatically reconnect to the queue manager when connection is lost, with configurable retry logic.

### Architecture

#### 1. Add Reconnection State Management

**File**: `RFHUtil/DataArea.h`

Add new member variables to the `DataArea` class (around line 470):

```cpp
// P0.2: Automatic reconnection settings
BOOL m_auto_reconnect;              // Enable/disable auto-reconnect
int m_reconnect_max_attempts;       // Maximum reconnection attempts (0 = infinite)
int m_reconnect_interval;           // Seconds between reconnection attempts
int m_reconnect_backoff_multiplier; // Backoff multiplier (1 = no backoff, 2 = double each time)
int m_reconnect_max_interval;       // Maximum interval between attempts (seconds)

// P0.2: Reconnection state tracking
int m_reconnect_attempt_count;      // Current reconnection attempt number
DWORD m_last_reconnect_time;        // Timestamp of last reconnection attempt
BOOL m_reconnecting;                // Flag indicating reconnection in progress
CString m_last_qm_name;             // Last connected QM name for reconnection
CString m_last_channel_name;        // Last used channel name
CString m_last_conn_name;           // Last used connection name
```

#### 2. Add Reconnection Method

**File**: `RFHUtil/DataArea.h`

Add method declaration (around line 628):

```cpp
bool attemptReconnection(LPCTSTR qmName, MQLONG failureReason);
void resetReconnectionState();
bool shouldAttemptReconnect(MQLONG rc);
int calculateReconnectDelay();
```

**File**: `RFHUtil/DataArea.cpp`

Add implementation after `connect2QM()` method (around line 10500):

```cpp
///////////////////////////////////////////////////////
//
// P0.2: Attempt automatic reconnection to queue manager
//
// This method implements exponential backoff retry logic
// with configurable maximum attempts and intervals.
//
///////////////////////////////////////////////////////

bool DataArea::attemptReconnection(LPCTSTR qmName, MQLONG failureReason)
{
	char traceInfo[512];
	
	// Check if auto-reconnect is enabled
	if (!m_auto_reconnect)
	{
		if (traceEnabled)
		{
			logTraceEntry("P0.2: Auto-reconnect disabled, not attempting reconnection");
		}
		return false;
	}
	
	// Check if we've exceeded maximum attempts
	if (m_reconnect_max_attempts > 0 && m_reconnect_attempt_count >= m_reconnect_max_attempts)
	{
		if (traceEnabled)
		{
			sprintf(traceInfo, "P0.2: Maximum reconnection attempts (%d) reached", m_reconnect_max_attempts);
			logTraceEntry(traceInfo);
		}
		m_reconnecting = false;
		return false;
	}
	
	// Calculate delay with exponential backoff
	int delay = calculateReconnectDelay();
	
	// Check if enough time has passed since last attempt
	DWORD currentTime = GetTickCount();
	DWORD timeSinceLastAttempt = currentTime - m_last_reconnect_time;
	
	if (timeSinceLastAttempt < (DWORD)(delay * 1000))
	{
		// Not enough time has passed, wait
		if (traceEnabled)
		{
			sprintf(traceInfo, "P0.2: Waiting %d ms before next reconnection attempt", 
					(delay * 1000) - timeSinceLastAttempt);
			logTraceEntry(traceInfo);
		}
		Sleep((delay * 1000) - timeSinceLastAttempt);
	}
	
	// Increment attempt counter
	m_reconnect_attempt_count++;
	m_last_reconnect_time = GetTickCount();
	m_reconnecting = true;
	
	if (traceEnabled)
	{
		sprintf(traceInfo, "P0.2: Reconnection attempt %d of %d (delay: %ds, reason: %d)", 
				m_reconnect_attempt_count, 
				m_reconnect_max_attempts > 0 ? m_reconnect_max_attempts : 999,
				delay,
				failureReason);
		logTraceEntry(traceInfo);
	}
	
	// Ensure clean state before reconnecting
	if (connected)
	{
		discQM();
	}
	
	// Attempt to reconnect
	bool success = connect2QM(qmName);
	
	if (success)
	{
		if (traceEnabled)
		{
			sprintf(traceInfo, "P0.2: Reconnection successful after %d attempts", m_reconnect_attempt_count);
			logTraceEntry(traceInfo);
		}
		
		// Reset reconnection state on success
		resetReconnectionState();
		
		// Show success message to user
		CString msg;
		msg.Format("Reconnected to queue manager '%s' after %d attempt(s)", 
				   qmName, m_reconnect_attempt_count);
		AfxMessageBox(msg, MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		if (traceEnabled)
		{
			sprintf(traceInfo, "P0.2: Reconnection attempt %d failed", m_reconnect_attempt_count);
			logTraceEntry(traceInfo);
		}
	}
	
	m_reconnecting = false;
	return success;
}

///////////////////////////////////////////////////////
//
// P0.2: Calculate reconnection delay with exponential backoff
//
///////////////////////////////////////////////////////

int DataArea::calculateReconnectDelay()
{
	int delay = m_reconnect_interval;
	
	// Apply exponential backoff
	if (m_reconnect_backoff_multiplier > 1 && m_reconnect_attempt_count > 0)
	{
		for (int i = 1; i < m_reconnect_attempt_count; i++)
		{
			delay *= m_reconnect_backoff_multiplier;
			
			// Cap at maximum interval
			if (delay > m_reconnect_max_interval)
			{
				delay = m_reconnect_max_interval;
				break;
			}
		}
	}
	
	return delay;
}

///////////////////////////////////////////////////////
//
// P0.2: Reset reconnection state
//
///////////////////////////////////////////////////////

void DataArea::resetReconnectionState()
{
	m_reconnect_attempt_count = 0;
	m_last_reconnect_time = 0;
	m_reconnecting = false;
}

///////////////////////////////////////////////////////
//
// P0.2: Check if reconnection should be attempted for this error
//
///////////////////////////////////////////////////////

bool DataArea::shouldAttemptReconnect(MQLONG rc)
{
	// Only attempt reconnection for connection-related errors
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
```

#### 3. Integrate Reconnection Logic

**File**: `RFHUtil/DataArea.cpp`

Modify existing error handling locations (7 places found):

**Location 1** (around line 7563):
```cpp
// check the return code for a 2009 error (connection to QMgr lost)
if (MQRC_CONNECTION_BROKEN == (*rc))
{
	// the connection is gone - make sure everything gets cleaned up
	discQM();
	
	// P0.2: Attempt automatic reconnection
	if (shouldAttemptReconnect(*rc))
	{
		if (attemptReconnection(m_QM_name, *rc))
		{
			// Reconnection successful, retry the operation
			// (caller should handle retry logic)
			return;
		}
	}
}
```

Apply similar changes to all 7 locations where connection errors are detected.

#### 4. Initialize Default Values

**File**: `RFHUtil/DataArea.cpp`

In the `DataArea` constructor (around line 200):

```cpp
// P0.2: Initialize reconnection settings with defaults
m_auto_reconnect = TRUE;                    // Enabled by default
m_reconnect_max_attempts = 3;               // Try 3 times
m_reconnect_interval = 5;                   // Start with 5 seconds
m_reconnect_backoff_multiplier = 2;         // Double each time
m_reconnect_max_interval = 60;              // Cap at 60 seconds

// P0.2: Initialize reconnection state
m_reconnect_attempt_count = 0;
m_last_reconnect_time = 0;
m_reconnecting = FALSE;
m_last_qm_name = "";
m_last_channel_name = "";
m_last_conn_name = "";
```

#### 5. Save Connection Parameters

**File**: `RFHUtil/DataArea.cpp`

In `connect2QM()` method, after successful connection (around line 10450):

```cpp
// P0.2: Save connection parameters for potential reconnection
m_last_qm_name = qm;
if (strlen(channelName) > 0)
{
	m_last_channel_name = channelName;
}
if (strlen(connName) > 0)
{
	m_last_conn_name = connName;
}

// P0.2: Reset reconnection state on successful connection
resetReconnectionState();
```

---

## P0.3: UI Tab for Connection Settings

### Objective
Add a new tab to the property sheet for configuring connection settings including HeartBeat, KeepAlive, and reconnection options.

### Architecture

#### 1. Create New Dialog Resource

**File**: `RFHUtil/rfhutil.rc`

Add new dialog resource (use Visual Studio Resource Editor):

```
IDD_CONN_SETTINGS DIALOGEX 0, 0, 320, 240
STYLE DS_SETFONT | DS_FIXEDSYS | WS_CHILD
FONT 8, "MS Shell Dlg", 400, 0, 0x1
BEGIN
    GROUPBOX        "HeartBeat Settings",IDC_STATIC,7,7,306,60
    LTEXT           "HeartBeat Interval (seconds):",IDC_STATIC,14,22,100,8
    EDITTEXT        IDC_HEARTBEAT_INTERVAL,120,20,40,14,ES_AUTOHSCROLL | ES_NUMBER
    LTEXT           "0 = Use QM default (300s)",IDC_STATIC,165,22,140,8
    CONTROL         "Enable HeartBeat",IDC_ENABLE_HEARTBEAT,"Button",BS_AUTOCHECKBOX | WS_TABSTOP,14,40,80,10
    LTEXT           "Recommended: 60 seconds for better connection monitoring",IDC_STATIC,14,52,280,8
    
    GROUPBOX        "KeepAlive Settings",IDC_STATIC,7,72,306,60
    LTEXT           "KeepAlive Interval:",IDC_STATIC,14,87,100,8
    COMBOBOX        IDC_KEEPALIVE_INTERVAL,120,85,100,60,CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP
    CONTROL         "Enable KeepAlive",IDC_ENABLE_KEEPALIVE,"Button",BS_AUTOCHECKBOX | WS_TABSTOP,14,105,80,10
    LTEXT           "AUTO = Use OS TCP/IP defaults (recommended)",IDC_STATIC,14,117,280,8
    
    GROUPBOX        "Automatic Reconnection",IDC_STATIC,7,137,306,96
    CONTROL         "Enable Automatic Reconnection",IDC_ENABLE_AUTO_RECONNECT,"Button",BS_AUTOCHECKBOX | WS_TABSTOP,14,152,120,10
    LTEXT           "Maximum Attempts:",IDC_STATIC,14,170,80,8
    EDITTEXT        IDC_RECONNECT_MAX_ATTEMPTS,100,168,40,14,ES_AUTOHSCROLL | ES_NUMBER
    LTEXT           "(0 = infinite)",IDC_STATIC,145,170,60,8
    LTEXT           "Initial Interval (seconds):",IDC_STATIC,14,188,80,8
    EDITTEXT        IDC_RECONNECT_INTERVAL,100,186,40,14,ES_AUTOHSCROLL | ES_NUMBER
    LTEXT           "Backoff Multiplier:",IDC_STATIC,14,206,80,8
    EDITTEXT        IDC_RECONNECT_BACKOFF,100,204,40,14,ES_AUTOHSCROLL | ES_NUMBER
    LTEXT           "(1 = no backoff, 2 = double)",IDC_STATIC,145,206,150,8
    LTEXT           "Maximum Interval (seconds):",IDC_STATIC,14,224,90,8
    EDITTEXT        IDC_RECONNECT_MAX_INTERVAL,110,222,40,14,ES_AUTOHSCROLL | ES_NUMBER
END
```

#### 2. Add Resource IDs

**File**: `RFHUtil/resource.h`

Add new control IDs:

```cpp
#define IDD_CONN_SETTINGS               2000
#define IDC_HEARTBEAT_INTERVAL          2001
#define IDC_ENABLE_HEARTBEAT            2002
#define IDC_KEEPALIVE_INTERVAL          2003
#define IDC_ENABLE_KEEPALIVE            2004
#define IDC_ENABLE_AUTO_RECONNECT       2005
#define IDC_RECONNECT_MAX_ATTEMPTS      2006
#define IDC_RECONNECT_INTERVAL          2007
#define IDC_RECONNECT_BACKOFF           2008
#define IDC_RECONNECT_MAX_INTERVAL      2009
```

#### 3. Create Dialog Class

**File**: `RFHUtil/ConnSettings.h` (NEW FILE)

```cpp
#pragma once

#include "DataArea.h"

class ConnSettings : public CPropertyPage
{
	DECLARE_DYNCREATE(ConnSettings)

public:
	ConnSettings();
	virtual ~ConnSettings();

	enum { IDD = IDD_CONN_SETTINGS };

	DataArea* pDoc;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual void OnOK();

	afx_msg void OnEnableHeartbeat();
	afx_msg void OnEnableKeepalive();
	afx_msg void OnEnableAutoReconnect();

	DECLARE_MESSAGE_MAP()

private:
	// HeartBeat settings
	BOOL m_enable_heartbeat;
	int m_heartbeat_interval;
	
	// KeepAlive settings
	BOOL m_enable_keepalive;
	CComboBox m_keepalive_combo;
	int m_keepalive_selection;
	
	// Reconnection settings
	BOOL m_enable_auto_reconnect;
	int m_reconnect_max_attempts;
	int m_reconnect_interval;
	int m_reconnect_backoff;
	int m_reconnect_max_interval;
	
	void UpdateControlStates();
	void LoadSettings();
	void SaveSettings();
};
```

**File**: `RFHUtil/ConnSettings.cpp` (NEW FILE)

```cpp
#include "stdafx.h"
#include "rfhutil.h"
#include "ConnSettings.h"

IMPLEMENT_DYNCREATE(ConnSettings, CPropertyPage)

ConnSettings::ConnSettings() : CPropertyPage(ConnSettings::IDD)
{
	pDoc = NULL;
	
	// Initialize with defaults
	m_enable_heartbeat = TRUE;
	m_heartbeat_interval = 60;
	m_enable_keepalive = TRUE;
	m_keepalive_selection = 0; // AUTO
	m_enable_auto_reconnect = TRUE;
	m_reconnect_max_attempts = 3;
	m_reconnect_interval = 5;
	m_reconnect_backoff = 2;
	m_reconnect_max_interval = 60;
}

ConnSettings::~ConnSettings()
{
}

void ConnSettings::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	
	DDX_Check(pDX, IDC_ENABLE_HEARTBEAT, m_enable_heartbeat);
	DDX_Text(pDX, IDC_HEARTBEAT_INTERVAL, m_heartbeat_interval);
	DDV_MinMaxInt(pDX, m_heartbeat_interval, 0, 999999);
	
	DDX_Check(pDX, IDC_ENABLE_KEEPALIVE, m_enable_keepalive);
	DDX_Control(pDX, IDC_KEEPALIVE_INTERVAL, m_keepalive_combo);
	DDX_CBIndex(pDX, IDC_KEEPALIVE_INTERVAL, m_keepalive_selection);
	
	DDX_Check(pDX, IDC_ENABLE_AUTO_RECONNECT, m_enable_auto_reconnect);
	DDX_Text(pDX, IDC_RECONNECT_MAX_ATTEMPTS, m_reconnect_max_attempts);
	DDV_MinMaxInt(pDX, m_reconnect_max_attempts, 0, 999);
	DDX_Text(pDX, IDC_RECONNECT_INTERVAL, m_reconnect_interval);
	DDV_MinMaxInt(pDX, m_reconnect_interval, 1, 3600);
	DDX_Text(pDX, IDC_RECONNECT_BACKOFF, m_reconnect_backoff);
	DDV_MinMaxInt(pDX, m_reconnect_backoff, 1, 10);
	DDX_Text(pDX, IDC_RECONNECT_MAX_INTERVAL, m_reconnect_max_interval);
	DDV_MinMaxInt(pDX, m_reconnect_max_interval, 1, 3600);
}

BEGIN_MESSAGE_MAP(ConnSettings, CPropertyPage)
	ON_BN_CLICKED(IDC_ENABLE_HEARTBEAT, &ConnSettings::OnEnableHeartbeat)
	ON_BN_CLICKED(IDC_ENABLE_KEEPALIVE, &ConnSettings::OnEnableKeepalive)
	ON_BN_CLICKED(IDC_ENABLE_AUTO_RECONNECT, &ConnSettings::OnEnableAutoReconnect)
END_MESSAGE_MAP()

BOOL ConnSettings::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	
	// Populate KeepAlive combo box
	m_keepalive_combo.AddString("AUTO (Use OS defaults)");
	m_keepalive_combo.AddString("Disabled");
	m_keepalive_combo.AddString("30 seconds");
	m_keepalive_combo.AddString("60 seconds");
	m_keepalive_combo.AddString("120 seconds");
	m_keepalive_combo.AddString("300 seconds");
	m_keepalive_combo.SetCurSel(0);
	
	LoadSettings();
	UpdateControlStates();
	
	return TRUE;
}

BOOL ConnSettings::OnSetActive()
{
	LoadSettings();
	UpdateData(FALSE);
	return CPropertyPage::OnSetActive();
}

void ConnSettings::OnOK()
{
	UpdateData(TRUE);
	SaveSettings();
	CPropertyPage::OnOK();
}

void ConnSettings::OnEnableHeartbeat()
{
	UpdateData(TRUE);
	UpdateControlStates();
}

void ConnSettings::OnEnableKeepalive()
{
	UpdateData(TRUE);
	UpdateControlStates();
}

void ConnSettings::OnEnableAutoReconnect()
{
	UpdateData(TRUE);
	UpdateControlStates();
}

void ConnSettings::UpdateControlStates()
{
	GetDlgItem(IDC_HEARTBEAT_INTERVAL)->EnableWindow(m_enable_heartbeat);
	GetDlgItem(IDC_KEEPALIVE_INTERVAL)->EnableWindow(m_enable_keepalive);
	
	BOOL enableReconnect = m_enable_auto_reconnect;
	GetDlgItem(IDC_RECONNECT_MAX_ATTEMPTS)->EnableWindow(enableReconnect);
	GetDlgItem(IDC_RECONNECT_INTERVAL)->EnableWindow(enableReconnect);
	GetDlgItem(IDC_RECONNECT_BACKOFF)->EnableWindow(enableReconnect);
	GetDlgItem(IDC_RECONNECT_MAX_INTERVAL)->EnableWindow(enableReconnect);
}

void ConnSettings::LoadSettings()
{
	if (pDoc == NULL)
		return;
	
	// Load HeartBeat settings
	m_enable_heartbeat = pDoc->m_enable_heartbeat;
	m_heartbeat_interval = pDoc->m_heartbeat_interval;
	
	// Load KeepAlive settings
	m_enable_keepalive = pDoc->m_enable_keepalive;
	m_keepalive_selection = pDoc->m_keepalive_selection;
	
	// Load reconnection settings
	m_enable_auto_reconnect = pDoc->m_auto_reconnect;
	m_reconnect_max_attempts = pDoc->m_reconnect_max_attempts;
	m_reconnect_interval = pDoc->m_reconnect_interval;
	m_reconnect_backoff = pDoc->m_reconnect_backoff_multiplier;
	m_reconnect_max_interval = pDoc->m_reconnect_max_interval;
}

void ConnSettings::SaveSettings()
{
	if (pDoc == NULL)
		return;
	
	// Save HeartBeat settings
	pDoc->m_enable_heartbeat = m_enable_heartbeat;
	pDoc->m_heartbeat_interval = m_heartbeat_interval;
	
	// Save KeepAlive settings
	pDoc->m_enable_keepalive = m_enable_keepalive;
	pDoc->m_keepalive_selection = m_keepalive_selection;
	
	// Save reconnection settings
	pDoc->m_auto_reconnect = m_enable_auto_reconnect;
	pDoc->m_reconnect_max_attempts = m_reconnect_max_attempts;
	pDoc->m_reconnect_interval = m_reconnect_interval;
	pDoc->m_reconnect_backoff_multiplier = m_reconnect_backoff;
	pDoc->m_reconnect_max_interval = m_reconnect_max_interval;
}
```

#### 4. Add New Variables to DataArea

**File**: `RFHUtil/DataArea.h`

Add around line 470:

```cpp
// P0.3: UI-configurable connection settings
BOOL m_enable_heartbeat;
int m_heartbeat_interval;
BOOL m_enable_keepalive;
int m_keepalive_selection;  // 0=AUTO, 1=Disabled, 2=30s, 3=60s, 4=120s, 5=300s
```

#### 5. Integrate into Property Sheet

**File**: `RFHUtil/MyPropertySheet.h`

Add include and member:

```cpp
#include "ConnSettings.h"

class CMyPropertySheet : public CPropertySheet
{
	// ... existing code ...
	
	ConnSettings m_conn_settings;  // P0.3: Connection settings tab
	
	// ... existing code ...
};
```

**File**: `RFHUtil/MyPropertySheet.cpp`

In constructor, add the new page:

```cpp
CMyPropertySheet::CMyPropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	// ... existing pages ...
	
	// P0.3: Add connection settings tab
	AddPage(&m_conn_settings);
}
```

#### 6. Update connect2QM() to Use UI Settings

**File**: `RFHUtil/DataArea.cpp`

In `connect2QM()` method (around line 10415):

```cpp
// P0.3: Apply HeartBeat settings from UI
if (m_enable_heartbeat && m_heartbeat_interval > 0)
{
	if (cd.Version < MQCD_VERSION_3)
	{
		cd.Version = MQCD_VERSION_3;
	}
	cd.HeartbeatInterval = m_heartbeat_interval;
	
	if (traceEnabled)
	{
		sprintf(traceInfo, "P0.3: HeartBeat enabled: %d seconds", m_heartbeat_interval);
		logTraceEntry(traceInfo);
	}
}

// P0.3: Apply KeepAlive settings from UI
if (m_enable_keepalive)
{
	if (cd.Version < MQCD_VERSION_6)
	{
		cd.Version = MQCD_VERSION_6;
	}
	
	// Map UI selection to MQKAI value
	switch (m_keepalive_selection)
	{
	case 0: // AUTO
		cd.KeepAliveInterval = MQKAI_AUTO;
		break;
	case 1: // Disabled
		cd.KeepAliveInterval = 0;
		break;
	case 2: // 30 seconds
		cd.KeepAliveInterval = 30;
		break;
	case 3: // 60 seconds
		cd.KeepAliveInterval = 60;
		break;
	case 4: // 120 seconds
		cd.KeepAliveInterval = 120;
		break;
	case 5: // 300 seconds
		cd.KeepAliveInterval = 300;
		break;
	}
	
	if (traceEnabled)
	{
		sprintf(traceInfo, "P0.3: KeepAlive enabled: selection=%d, value=%d", 
				m_keepalive_selection, cd.KeepAliveInterval);
		logTraceEntry(traceInfo);
	}
}
```

---

## Testing Plan

### P0.2 Testing

1. **Basic Reconnection**
   - Connect to QM
   - Stop QM
   - Verify automatic reconnection attempts
   - Start QM
   - Verify successful reconnection

2. **Exponential Backoff**
   - Monitor reconnection intervals
   - Verify backoff multiplier works
   - Verify maximum interval cap

3. **Maximum Attempts**
   - Set max attempts to 3
   - Stop QM permanently
   - Verify it stops after 3 attempts

4. **Disable Auto-Reconnect**
   - Disable feature
   - Stop QM
   - Verify no reconnection attempts

### P0.3 Testing

1. **HeartBeat Configuration**
   - Set different intervals (30s, 60s, 120s)
   - Verify MQCD version upgraded
   - Check MQ logs for heartbeat activity

2. **KeepAlive Configuration**
   - Test AUTO setting
   - Test disabled
   - Test specific intervals
   - Verify TCP keepalive packets

3. **UI Persistence**
   - Set values
   - Close and reopen application
   - Verify settings persisted

4. **Integration**
   - Configure all settings
   - Test connection
   - Verify all settings applied

---

## Configuration Files

### Registry Settings (Optional Enhancement)

Store settings in registry for persistence:

```
HKEY_CURRENT_USER\Software\RFHUtil\ConnectionSettings
- HeartBeatEnabled (DWORD)
- HeartBeatInterval (DWORD)
- KeepAliveEnabled (DWORD)
- KeepAliveSelection (DWORD)
- AutoReconnectEnabled (DWORD)
- ReconnectMaxAttempts (DWORD)
- ReconnectInterval (DWORD)
- ReconnectBackoff (DWORD)
- ReconnectMaxInterval (DWORD)
```

---

## Error Handling

### Connection Errors to Handle

```cpp
MQRC_CONNECTION_BROKEN       (2009) - Connection lost
MQRC_Q_MGR_NOT_AVAILABLE     (2059) - QM not available
MQRC_CONNECTION_QUIESCING    (2161) - Connection shutting down
MQRC_CONNECTION_STOPPED      (2162) - Connection stopped
MQRC_HCONN_ERROR             (2018) - Invalid connection handle
MQRC_Q_MGR_STOPPING          (2162) - QM stopping
```

### User Notifications

1. **Reconnection Started**
   - Show status in status bar
   - Log to trace file

2. **Reconnection Successful**
   - Show message box (optional)
   - Update status bar
   - Log success

3. **Reconnection Failed**
   - Show error message
   - Provide manual reconnect option
   - Log failure details

---

## Performance Considerations

1. **UI Responsiveness**
   - Run reconnection in background thread
   - Don't block UI during reconnection
   - Show progress indicator

2. **Resource Management**
   - Clean up handles properly
   - Avoid memory leaks during reconnection
   - Limit concurrent reconnection attempts

3. **Network Efficiency**
   - Use appropriate intervals
   - Implement exponential backoff
   - Cap maximum interval

---

## Documentation Updates

### User Documentation

Update README.md with:
- Connection settings tab description
- HeartBeat/KeepAlive explanation
- Automatic reconnection feature
- Recommended settings

### Developer Documentation

Update ARCHITECTURE_ANALYSIS.md with:
- New reconnection architecture
- UI integration details
- State management approach

---

## Estimated Effort

### P0.2: Automatic Reconnection
- Core logic: 6 hours
- Integration: 4 hours
- Testing: 4 hours
- Documentation: 2 hours
- Buffer: 2 hours
**Total: 18 hours**

### P0.3: UI Tab
- Dialog design: 2 hours
- Class implementation: 4 hours
- Integration: 3 hours
- Testing: 2 hours
- Documentation: 1 hour
**Total: 12 hours**

### Combined Total: 30 hours

---

## Success Criteria

### P0.2
✅ Automatic reconnection works after connection loss
✅ Exponential backoff implemented correctly
✅ Maximum attempts respected
✅ User can enable/disable feature
✅ Trace logging shows reconnection activity

### P0.3
✅ New tab appears in property sheet
✅ All settings configurable via UI
✅ Settings persist across sessions
✅ Settings applied to connections
✅ UI updates reflect current state

---

## Next Steps

1. Review this implementation guide
2. Allocate development time (30 hours)
3. Implement P0.2 first (more critical)
4. Test P0.2 thoroughly
5. Implement P0.3
6. Integration testing
7. User acceptance testing
8. Documentation updates
9. Release

---

## Notes

- This implementation maintains backward compatibility
- All features are optional and can be disabled
- Default settings provide good balance of reliability and performance
- Trace logging helps with troubleshooting
- UI is intuitive and follows existing patterns

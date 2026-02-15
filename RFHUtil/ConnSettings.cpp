/*
Copyright (c) IBM Corporation 2000, 2026
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Contributors:
Jim MacNair - Initial Contribution
Bob (AI Assistant) - P0.3 Connection Settings Implementation
*/

// ConnSettings.cpp : implementation file
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "rfhutil.h"
#include "ThemeManager.h"
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
	m_enable_heartbeat = TRUE;
	m_heartbeat_interval = 60;
	m_enable_keepalive = TRUE;
	m_keepalive_selection = 0;  // AUTO
	m_enable_auto_reconnect = TRUE;
	m_reconnect_max_attempts = 3;
	m_reconnect_interval = 5;
	m_reconnect_backoff = 2;
	m_reconnect_max_interval = 60;
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
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(ConnSettings, CPropertyPage)
	//{{AFX_MSG_MAP(ConnSettings)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_ENABLE_HEARTBEAT, &ConnSettings::OnEnableHeartbeat)
	ON_BN_CLICKED(IDC_ENABLE_KEEPALIVE, &ConnSettings::OnEnableKeepalive)
	ON_BN_CLICKED(IDC_ENABLE_AUTO_RECONNECT, &ConnSettings::OnEnableAutoReconnect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ConnSettings message handlers

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
	m_keepalive_combo.AddString("600 seconds");
	
	// Set initial selection
	m_keepalive_combo.SetCurSel(m_keepalive_selection);
	
	// Load settings from DataArea
	LoadSettings();
	
	// Update control states based on checkboxes
	UpdateControlStates();
	
	
	// Apply theme to dialog
	ThemeManager::GetInstance().ApplyThemeToDialog(this);

	return TRUE;
}

BOOL ConnSettings::OnSetActive()
{
	BOOL ret = CPropertyPage::OnSetActive();
	
	// Reload settings when page becomes active
	LoadSettings();
	UpdateData(FALSE);
	UpdateControlStates();
	
	return ret;
}

void ConnSettings::OnOK()
{
	// Save settings to DataArea
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
	// Enable/disable HeartBeat interval based on checkbox
	GetDlgItem(IDC_HEARTBEAT_INTERVAL)->EnableWindow(m_enable_heartbeat);
	
	// Enable/disable KeepAlive combo based on checkbox
	GetDlgItem(IDC_KEEPALIVE_INTERVAL)->EnableWindow(m_enable_keepalive);
	
	// Enable/disable reconnection controls based on checkbox
	GetDlgItem(IDC_RECONNECT_MAX_ATTEMPTS)->EnableWindow(m_enable_auto_reconnect);
	GetDlgItem(IDC_RECONNECT_INTERVAL)->EnableWindow(m_enable_auto_reconnect);
	GetDlgItem(IDC_RECONNECT_BACKOFF)->EnableWindow(m_enable_auto_reconnect);
	GetDlgItem(IDC_RECONNECT_MAX_INTERVAL)->EnableWindow(m_enable_auto_reconnect);
}

void ConnSettings::LoadSettings()
{
	if (pDoc == NULL)
		return;
	
	// Load HeartBeat settings
	m_enable_heartbeat = pDoc->m_heartbeat_enabled;
	m_heartbeat_interval = pDoc->m_heartbeat_interval;
	
	// Load KeepAlive settings
	m_enable_keepalive = pDoc->m_keepalive_enabled;
	
	// Convert KeepAlive interval to combo box selection
	if (pDoc->m_keepalive_interval == -1)  // MQKAI_AUTO
		m_keepalive_selection = 0;  // AUTO
	else if (pDoc->m_keepalive_interval == 0)
		m_keepalive_selection = 1;  // Disabled
	else if (pDoc->m_keepalive_interval == 30)
		m_keepalive_selection = 2;
	else if (pDoc->m_keepalive_interval == 60)
		m_keepalive_selection = 3;
	else if (pDoc->m_keepalive_interval == 120)
		m_keepalive_selection = 4;
	else if (pDoc->m_keepalive_interval == 300)
		m_keepalive_selection = 5;
	else if (pDoc->m_keepalive_interval == 600)
		m_keepalive_selection = 6;
	else
		m_keepalive_selection = 0;  // Default to AUTO
	
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
	pDoc->m_heartbeat_enabled = m_enable_heartbeat;
	pDoc->m_heartbeat_interval = m_heartbeat_interval;
	
	// Save KeepAlive settings
	pDoc->m_keepalive_enabled = m_enable_keepalive;
	
	// Convert combo box selection to KeepAlive interval
	switch (m_keepalive_selection)
	{
	case 0:  // AUTO
		pDoc->m_keepalive_interval = -1;  // MQKAI_AUTO
		break;
	case 1:  // Disabled
		pDoc->m_keepalive_interval = 0;
		break;
	case 2:  // 30 seconds
		pDoc->m_keepalive_interval = 30;
		break;
	case 3:  // 60 seconds
		pDoc->m_keepalive_interval = 60;
		break;
	case 4:  // 120 seconds
		pDoc->m_keepalive_interval = 120;
		break;
	case 5:  // 300 seconds
		pDoc->m_keepalive_interval = 300;
		break;
	case 6:  // 600 seconds
		pDoc->m_keepalive_interval = 600;
		break;
	default:
		pDoc->m_keepalive_interval = -1;  // MQKAI_AUTO
		break;
	}
	
	// Save reconnection settings
	pDoc->m_auto_reconnect = m_enable_auto_reconnect;
	pDoc->m_reconnect_max_attempts = m_reconnect_max_attempts;
	pDoc->m_reconnect_interval = m_reconnect_interval;
	pDoc->m_reconnect_backoff_multiplier = m_reconnect_backoff;
	pDoc->m_reconnect_max_interval = m_reconnect_max_interval;
}

void ConnSettings::UpdatePageData()
{
	if (pDoc != NULL)
	{
		// Reload settings from DataArea
		LoadSettings();
		
		// Update the dialog controls
		UpdateData(FALSE);
		
		// Update control states
		UpdateControlStates();
	}
}

HBRUSH ConnSettings::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	ThemeManager& theme = ThemeManager::GetInstance();
	
	if (theme.IsDarkMode()) {
		pDC->SetTextColor(theme.GetTextColor());
		
		switch (nCtlColor) {
			case CTLCOLOR_EDIT:
			case CTLCOLOR_LISTBOX:
				// Dark grey background for edit controls and combo boxes
				pDC->SetBkColor(theme.GetControlBackgroundColor());
				return (HBRUSH)theme.GetControlBackgroundBrush()->GetSafeHandle();
				
			case CTLCOLOR_STATIC:
				// Dialog background for static text
				pDC->SetBkColor(theme.GetBackgroundColor());
				return (HBRUSH)theme.GetBackgroundBrush()->GetSafeHandle();
				
			case CTLCOLOR_BTN:
				// Button background
				pDC->SetBkColor(theme.GetButtonBackgroundColor());
				return (HBRUSH)theme.GetControlBackgroundBrush()->GetSafeHandle();
				
			case CTLCOLOR_DLG:
				// Dialog background
				return (HBRUSH)theme.GetBackgroundBrush()->GetSafeHandle();
		}
	}
	
	return CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL ConnSettings::OnEraseBkgnd(CDC* pDC)
{
	ThemeManager& theme = ThemeManager::GetInstance();
	
	if (theme.IsDarkMode()) {
		CRect rect;
		GetClientRect(&rect);
		theme.DrawGradientBackground(pDC, rect);
		return TRUE;
	}
	
	return CPropertyPage::OnEraseBkgnd(pDC);
}

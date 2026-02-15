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

#if !defined(AFX_CONNSETTINGS_H__INCLUDED_)
#define AFX_CONNSETTINGS_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// ConnSettings dialog - P0.3 Connection Settings Tab
//
// This property page provides UI controls for configuring:
// - HeartBeat interval settings
// - KeepAlive interval settings  
// - Automatic reconnection parameters
//
/////////////////////////////////////////////////////////////////////////////

#include "resource.h"
#include "DataArea.h"

class ConnSettings : public CPropertyPage
{
	DECLARE_DYNCREATE(ConnSettings)

// Construction
public:
	ConnSettings();
	~ConnSettings();
	
	DataArea* pDoc;
	void UpdatePageData();

// Dialog Data
	//{{AFX_DATA(ConnSettings)
	enum { IDD = IDD_CONN_SETTINGS };
	
	// HeartBeat settings
	BOOL	m_enable_heartbeat;
	int		m_heartbeat_interval;
	
	// KeepAlive settings
	BOOL	m_enable_keepalive;
	CComboBox m_keepalive_combo;
	int		m_keepalive_selection;
	
	// Reconnection settings
	BOOL	m_enable_auto_reconnect;
	int		m_reconnect_max_attempts;
	int		m_reconnect_interval;
	int		m_reconnect_backoff;
	int		m_reconnect_max_interval;
	//}}AFX_DATA

// Overrides
	//{{AFX_VIRTUAL(ConnSettings)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(ConnSettings)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnEnableHeartbeat();
	afx_msg void OnEnableKeepalive();
	afx_msg void OnEnableAutoReconnect();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void UpdateControlStates();
	void LoadSettings();
	void SaveSettings();
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CONNSETTINGS_H__INCLUDED_)
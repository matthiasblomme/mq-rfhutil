// MyPropertySheet.cpp : implementation file
//

#include "stdafx.h"
#include "rfhutil.h"
#include "MyPropertySheet.h"
#include "ThemeManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MyPropertySheet

MyPropertySheet::MyPropertySheet()

{
	CDC		*dc = NULL;
	HDC		hdc = NULL;

	m_wFontSize = 14;
	dpi = 0;
	m_pszFontFaceName = "Courier New";

	// try to get a device context
	dc = GetDC();

	if (dc != NULL)
	{
		hdc = dc->GetSafeHdc();
	}

	// check if it worked
	if (hdc != NULL)
	{
		// get the horizontal resolution as dpi
		dpi = GetDeviceCaps(hdc, LOGPIXELSX);

		// check for a higher resolution screen
		if (dpi > 100)
		{
			// use a larger font size
			m_wFontSize = (m_wFontSize * dpi) / 100;
		}
	}
}

MyPropertySheet::~MyPropertySheet()
{
}


BEGIN_MESSAGE_MAP(MyPropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(MyPropertySheet)
	ON_WM_KEYDOWN()
    ON_CONTROL_REFLECT(CBN_EDITUPDATE, OnEditUpdate)
	//}}AFX_MSG_MAP
	ON_WM_DRAWITEM()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MyPropertySheet callback routine

int CALLBACK MyPropertySheet::PropSheetProc(HWND hWndDlg, UINT uMsg, LPARAM lParam)
{
   switch(uMsg)
   {
   case PSCB_PRECREATE: // property sheet is being created
      {
           LPDLGTEMPLATE pResource = (LPDLGTEMPLATE)lParam;
           CDialogTemplate dlgTemplate(pResource);
           dlgTemplate.SetFont(m_pszFontFaceName, m_wFontSize);
           memmove((void*)lParam, dlgTemplate.m_hTemplate, dlgTemplate.m_dwTemplateSize);
      }
      break;
   }

   return 0;
}

/////////////////////////////////////////////////////////////////////////////
// MyPropertySheet message handlers

int MyPropertySheet::DoModal() 
{
    m_psh.dwFlags |= PSH_USECALLBACK;
    m_psh.pfnCallback = PropSheetProc;

    return CPropertySheet::DoModal();
}

void MyPropertySheet::BuildPropPageArray()
{
    CPropertySheet::BuildPropPageArray();

	PROPSHEETPAGE* ppsp = ( PROPSHEETPAGE* )m_psh.ppsp;
    for ( int nPage = 0; nPage < m_pages.GetSize(); nPage++ )
    {
        LPDLGTEMPLATE pTemplate = ( LPDLGTEMPLATE )ppsp[ nPage ].pResource;

		DialogTemplate dlgTemplate;
		DLGTEMPLATE dlgTemplate;
		dlgTemplate.Attach( pTemplate );
        dlgTemplate.SetFont(m_pszFontFaceName, m_wFontSize);
        dlgTemplate.Detach();
    }
}

/*int CALLBACK MyPropertySheet::PropSheetProc( HWND hwndDlg, UINT uMsg, LPARAM lParam )
{
    switch ( uMsg )
    {
        case PSCB_PRECREATE:
        {
            LPDLGTEMPLATE pTemplate = ( LPDLGTEMPLATE )lParam;

            CMyDialogTemplate dlgTemplate;
            dlgTemplate.Attach( pTemplate );
            dlgTemplate.SetFont( lpszFace, wSize );
            dlgTemplate.Detach();
            break;
        }
        default:
            break;
    }

    return 0;
}*/

/*void MyDialogTemplate::Attach( LPDLGTEMPLATE pTemplate )
{
    m_hTemplate      = ::GlobalHandle( pTemplate );
    m_dwTemplateSize = GetTemplateSize( pTemplate );
    m_bSystemFont    = false;
}*/

BOOL MyPropertySheet::OnInitDialog()
{
	BOOL result = CPropertySheet::OnInitDialog();
	
	// Apply theme to property sheet and tab control
	ThemeManager::GetInstance().ApplyThemeToPropertySheet(this);
	
	return result;
}

void MyPropertySheet::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// Check if this is the tab control
	CTabCtrl* pTab = GetTabControl();
	if (pTab && pTab->GetSafeHwnd() == lpDrawItemStruct->hwndItem) {
		ThemeManager& theme = ThemeManager::GetInstance();
		
		if (!theme.IsDarkMode()) {
			CPropertySheet::OnDrawItem(nIDCtl, lpDrawItemStruct);
			return;
		}
		
		// Get tab text
		TCITEM tci;
		char szText[256];
		tci.mask = TCIF_TEXT;
		tci.pszText = szText;
		tci.cchTextMax = sizeof(szText);
		pTab->GetItem(lpDrawItemStruct->itemID, &tci);
		
		// Determine if tab is selected
		int curSel = pTab->GetCurSel();
		bool selected = (lpDrawItemStruct->itemID == (UINT)curSel);
		bool hot = (lpDrawItemStruct->itemState & ODS_HOTLIGHT) != 0;
		
		// Draw the tab
		CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
		CRect rect = lpDrawItemStruct->rcItem;
		theme.DrawThemedTab(pDC, rect, szText, selected, hot);
		
		return;
	}
	
	CPropertySheet::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

HBRUSH MyPropertySheet::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	ThemeManager& theme = ThemeManager::GetInstance();
	
	if (theme.IsDarkMode()) {
		pDC->SetTextColor(theme.GetTextColor());
		pDC->SetBkColor(theme.GetBackgroundColor());
		
		if (nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC) {
			return *theme.GetBackgroundBrush();
		}
		else if (nCtlColor == CTLCOLOR_SCROLLBAR) {
			return *theme.GetControlBackgroundBrush();
		}
	}
	
	return CPropertySheet::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL MyPropertySheet::OnEraseBkgnd(CDC* pDC)
{
	ThemeManager& theme = ThemeManager::GetInstance();
	
	if (theme.IsDarkMode()) {
		// Get the entire client area
		CRect rect;
		GetClientRect(&rect);
		
		// Fill the entire property sheet background with dark color
		pDC->FillSolidRect(rect, theme.GetBackgroundColor());
		
		// Specifically fill the tab control area to ensure it's dark
		CTabCtrl* pTab = GetTabControl();
		if (pTab && pTab->GetSafeHwnd()) {
			CRect tabRect;
			pTab->GetWindowRect(&tabRect);
			ScreenToClient(&tabRect);
			
			// Fill entire tab control area with dark background
			pDC->FillSolidRect(tabRect, theme.GetBackgroundColor());
			
			// Get the display area (where pages show)
			CRect displayRect;
			pTab->GetItemRect(0, &displayRect);
			pTab->AdjustRect(FALSE, &displayRect);
			
			// Fill the tab strip area (above the display area)
			CRect tabStripRect = tabRect;
			tabStripRect.bottom = displayRect.top;
			pDC->FillSolidRect(tabStripRect, theme.GetBackgroundColor());
			
			// Also paint directly on the tab control's DC
			CDC* pTabDC = pTab->GetDC();
			if (pTabDC) {
				CRect tabClientRect;
				pTab->GetClientRect(&tabClientRect);
				pTabDC->FillSolidRect(tabClientRect, theme.GetBackgroundColor());
				pTab->ReleaseDC(pTabDC);
			}
		}
		
		return TRUE;
	}
	
	return CPropertySheet::OnEraseBkgnd(pDC);
}

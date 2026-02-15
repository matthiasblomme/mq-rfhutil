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
Bob (AI Assistant) - P1.2 Dark Mode Implementation
*/

// ThemeManager.cpp
// Implementation of theme management
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ThemeManager.h"
#include "rfhutil.h"
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

#include <uxtheme.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment(lib, "uxtheme.lib")

ThemeManager::ThemeManager()
    : m_currentTheme(AppTheme::LIGHT)
{
    // Initialize light color scheme
    m_lightColors.background = RGB(255, 255, 255);          // White
    m_lightColors.text = RGB(0, 0, 0);                      // Black
    m_lightColors.controlBackground = RGB(240, 240, 240);   // Light gray
    m_lightColors.border = RGB(200, 200, 200);              // Gray
    m_lightColors.highlight = RGB(0, 120, 215);             // Blue
    m_lightColors.disabledText = RGB(128, 128, 128);        // Gray
    m_lightColors.buttonBackground = RGB(240, 240, 240);    // Light gray
    m_lightColors.buttonBorder = RGB(200, 200, 200);        // Gray
    m_lightColors.gradientStart = RGB(255, 255, 255);       // White
    m_lightColors.gradientEnd = RGB(245, 245, 245);         // Very light gray
    
    // Initialize dark color scheme - Enhanced for better visual appearance
    m_darkColors.background = RGB(30, 30, 30);              // Darker background
    m_darkColors.text = RGB(240, 240, 240);                 // Slightly off-white for less eye strain
    m_darkColors.controlBackground = RGB(48, 48, 48);       // Dark grey for fields (better contrast)
    m_darkColors.border = RGB(70, 70, 70);                  // Lighter border for visibility
    m_darkColors.highlight = RGB(0, 120, 215);              // Blue (same as light)
    m_darkColors.disabledText = RGB(120, 120, 120);         // Slightly lighter gray
    m_darkColors.buttonBackground = RGB(55, 55, 55);        // Dark grey for buttons
    m_darkColors.buttonBorder = RGB(80, 80, 80);            // Lighter border for buttons
    m_darkColors.gradientStart = RGB(35, 35, 35);           // Slightly lighter than background
    m_darkColors.gradientEnd = RGB(25, 25, 25);             // Slightly darker than background
    
    // Create initial brushes
    UpdateBrushes();
}

ThemeManager::~ThemeManager()
{
    // Brushes will be automatically cleaned up by CBrush destructor
}

ThemeManager& ThemeManager::GetInstance()
{
    static ThemeManager instance;
    return instance;
}

void ThemeManager::SetTheme(AppTheme theme)
{
	if (m_currentTheme == theme) {
		return; // No change
	}
	
	m_currentTheme = theme;
	UpdateBrushes();
	SaveThemePreference();
	
	// Notify all windows to repaint
	CRfhutilApp* app = (CRfhutilApp*)AfxGetApp();
	if (app && app->m_pMainWnd) {
		// Apply dark title bar
		ApplyDarkTitleBar(app->m_pMainWnd);
		
		// Redraw everything
		app->m_pMainWnd->RedrawWindow(NULL, NULL,
			RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW);
	}
}

bool ThemeManager::IsDarkMode() const
{
    if (m_currentTheme == AppTheme::SYSTEM) {
        return IsSystemDarkMode();
    }
    return m_currentTheme == AppTheme::DARK;
}

const ThemeManager::ColorScheme& ThemeManager::GetCurrentColors() const
{
    return IsDarkMode() ? m_darkColors : m_lightColors;
}

COLORREF ThemeManager::GetBackgroundColor() const
{
    return GetCurrentColors().background;
}

COLORREF ThemeManager::GetTextColor() const
{
    return GetCurrentColors().text;
}

COLORREF ThemeManager::GetControlBackgroundColor() const
{
    return GetCurrentColors().controlBackground;
}

COLORREF ThemeManager::GetBorderColor() const
{
    return GetCurrentColors().border;
}

COLORREF ThemeManager::GetHighlightColor() const
{
    return GetCurrentColors().highlight;
}

COLORREF ThemeManager::GetDisabledTextColor() const
{
	return GetCurrentColors().disabledText;
}

COLORREF ThemeManager::GetButtonBackgroundColor() const
{
	return GetCurrentColors().buttonBackground;
}

COLORREF ThemeManager::GetButtonBorderColor() const
{
	return GetCurrentColors().buttonBorder;
}

COLORREF ThemeManager::GetGradientStartColor() const
{
	return GetCurrentColors().gradientStart;
}

COLORREF ThemeManager::GetGradientEndColor() const
{
	return GetCurrentColors().gradientEnd;
}

void ThemeManager::UpdateBrushes()
{
    // Delete old brushes
    if (m_backgroundBrush.GetSafeHandle()) {
        m_backgroundBrush.DeleteObject();
    }
    if (m_controlBackgroundBrush.GetSafeHandle()) {
        m_controlBackgroundBrush.DeleteObject();
    }
    
    // Create new brushes with current colors
    m_backgroundBrush.CreateSolidBrush(GetBackgroundColor());
    m_controlBackgroundBrush.CreateSolidBrush(GetControlBackgroundColor());
}

CBrush* ThemeManager::GetBackgroundBrush()
{
    return &m_backgroundBrush;
}

CBrush* ThemeManager::GetControlBackgroundBrush()
{
    return &m_controlBackgroundBrush;
}

void ThemeManager::ApplyThemeToDialog(CDialog* pDialog)
{
    if (!pDialog || !pDialog->GetSafeHwnd()) {
        return;
    }
    
    // Force dialog to repaint with new colors
    pDialog->Invalidate(TRUE);
    pDialog->UpdateWindow();
    
    // Apply to all child controls
    CWnd* pChild = pDialog->GetWindow(GW_CHILD);
    while (pChild) {
        ApplyThemeToControl(pChild);
        pChild = pChild->GetWindow(GW_HWNDNEXT);
    }
}

void ThemeManager::ApplyThemeToControl(CWnd* pControl)
{
    if (!pControl || !pControl->GetSafeHwnd()) {
        return;
    }
    
    // Force control to repaint
    pControl->Invalidate(TRUE);
    pControl->UpdateWindow();
}

void ThemeManager::LoadThemePreference()
{
    CRfhutilApp* app = (CRfhutilApp*)AfxGetApp();
    if (!app) {
        return;
    }
    
    // Load from registry or ini file
    int themeValue = app->GetProfileInt("Settings", "Theme", (int)AppTheme::LIGHT);
    
    // Validate the value
    if (themeValue < 0 || themeValue > 2) {
        themeValue = (int)AppTheme::LIGHT;
    }
    
    m_currentTheme = (AppTheme)themeValue;
    UpdateBrushes();
}

void ThemeManager::SaveThemePreference()
{
    CRfhutilApp* app = (CRfhutilApp*)AfxGetApp();
    if (!app) {
        return;
    }
    
    // Save to registry or ini file
    app->WriteProfileInt("Settings", "Theme", (int)m_currentTheme);
}

bool ThemeManager::IsSystemDarkMode() const
{
    // Check Windows 10+ dark mode setting
    // HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize
    // AppsUseLightTheme = 0 means dark mode
    
    HKEY hKey;
    DWORD value = 1; // Default to light mode
    DWORD size = sizeof(DWORD);
    
    if (RegOpenKeyEx(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        
        RegQueryValueEx(hKey, "AppsUseLightTheme", NULL, NULL,
            (LPBYTE)&value, &size);
        RegCloseKey(hKey);
    }
    
    return (value == 0); // 0 = dark mode, 1 = light mode
}

void ThemeManager::DrawGradientBackground(CDC* pDC, CRect rect)
{
	if (!IsDarkMode()) {
		// Light mode - solid background
		pDC->FillSolidRect(rect, GetBackgroundColor());
		return;
	}
	
	// Dark mode - vertical gradient for better aesthetics
	COLORREF startColor = GetGradientStartColor();
	COLORREF endColor = GetGradientEndColor();
	
	// Create gradient using GradientFill (Windows 2000+)
	TRIVERTEX vertex[2];
	vertex[0].x = rect.left;
	vertex[0].y = rect.top;
	vertex[0].Red = GetRValue(startColor) << 8;
	vertex[0].Green = GetGValue(startColor) << 8;
	vertex[0].Blue = GetBValue(startColor) << 8;
	vertex[0].Alpha = 0x0000;
	
	vertex[1].x = rect.right;
	vertex[1].y = rect.bottom;
	vertex[1].Red = GetRValue(endColor) << 8;
	vertex[1].Green = GetGValue(endColor) << 8;
	vertex[1].Blue = GetBValue(endColor) << 8;
	vertex[1].Alpha = 0x0000;
	
	GRADIENT_RECT gRect;
	gRect.UpperLeft = 0;
	gRect.LowerRight = 1;
	
	pDC->GradientFill(vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
}

void ThemeManager::DrawControlBorder(CDC* pDC, CRect rect)
{
	if (!IsDarkMode()) {
		return; // No custom borders in light mode
	}
	
	// Draw subtle border for visual separation
	CPen borderPen(PS_SOLID, 1, GetBorderColor());
	CPen* oldPen = pDC->SelectObject(&borderPen);
	
	pDC->MoveTo(rect.left, rect.top);
	pDC->LineTo(rect.right - 1, rect.top);
	pDC->LineTo(rect.right - 1, rect.bottom - 1);
	pDC->LineTo(rect.left, rect.bottom - 1);
	pDC->LineTo(rect.left, rect.top);
	
	pDC->SelectObject(oldPen);
}

void ThemeManager::DrawThemedButton(CDC* pDC, CRect rect, LPCTSTR text, UINT state)
{
	if (!IsDarkMode()) {
		// Use default drawing for light mode
		return;
	}
	
	// Dark mode button drawing
	COLORREF bgColor = GetButtonBackgroundColor();
	COLORREF borderColor = GetButtonBorderColor();
	COLORREF textColor = GetTextColor();
	
	// Adjust colors based on button state
	if (state & ODS_SELECTED) {
		// Button is pressed - darken background
		bgColor = RGB(
			max(0, GetRValue(bgColor) - 15),
			max(0, GetGValue(bgColor) - 15),
			max(0, GetBValue(bgColor) - 15)
		);
	} else if (state & ODS_FOCUS) {
		// Button has focus - use highlight color for border
		borderColor = GetHighlightColor();
	}
	
	// Draw button background
	CBrush bgBrush(bgColor);
	pDC->FillRect(rect, &bgBrush);
	
	// Draw border
	CPen borderPen(PS_SOLID, 1, borderColor);
	CPen* oldPen = pDC->SelectObject(&borderPen);
	pDC->Rectangle(rect);
	pDC->SelectObject(oldPen);
	
	// Draw text
	pDC->SetTextColor(textColor);
	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(text, -1, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

void ThemeManager::ApplyDarkTitleBar(CWnd* pWnd)
{
	if (!pWnd || !pWnd->GetSafeHwnd()) {
		return;
	}
	
	// Only apply dark title bar in dark mode
	if (!IsDarkMode()) {
		// Revert to light title bar
		BOOL useDarkMode = FALSE;
		DwmSetWindowAttribute(pWnd->GetSafeHwnd(), 
			20, // DWMWA_USE_IMMERSIVE_DARK_MODE (Windows 11) or 19 (Windows 10 build 18985+)
			&useDarkMode, sizeof(useDarkMode));
		return;
	}
	
	// Apply dark title bar for Windows 10/11
	BOOL useDarkMode = TRUE;
	
	// Try Windows 11 attribute first (DWMWA_USE_IMMERSIVE_DARK_MODE = 20)
	HRESULT hr = DwmSetWindowAttribute(pWnd->GetSafeHwnd(), 
		20, 
		&useDarkMode, sizeof(useDarkMode));
	
	// If that fails, try Windows 10 build 18985+ attribute (19)
	if (FAILED(hr)) {
		DwmSetWindowAttribute(pWnd->GetSafeHwnd(), 
			19, 
			&useDarkMode, sizeof(useDarkMode));
	}
	
	// Force window to redraw title bar
	SetWindowPos(pWnd->GetSafeHwnd(), NULL, 0, 0, 0, 0,
		SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
}
}
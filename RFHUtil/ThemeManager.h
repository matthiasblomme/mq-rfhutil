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

// ThemeManager.h
// Manages application theme (Light/Dark mode)
//////////////////////////////////////////////////////////////////////

#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <afxwin.h>

enum class AppTheme {
    LIGHT = 0,
    DARK = 1,
    SYSTEM = 2  // Follow Windows system theme
};

class ThemeManager {
public:
    static ThemeManager& GetInstance();
    
    // Theme management
    void SetTheme(AppTheme theme);
    AppTheme GetTheme() const { return m_currentTheme; }
    bool IsDarkMode() const;
    
    // Color getters
    COLORREF GetBackgroundColor() const;
    COLORREF GetTextColor() const;
    COLORREF GetControlBackgroundColor() const;
    COLORREF GetBorderColor() const;
    COLORREF GetHighlightColor() const;
    COLORREF GetDisabledTextColor() const;
    COLORREF GetButtonBackgroundColor() const;
    COLORREF GetButtonBorderColor() const;
    COLORREF GetGradientStartColor() const;
    COLORREF GetGradientEndColor() const;
    
    // Apply theme to controls
    void ApplyThemeToDialog(CDialog* pDialog);
    void ApplyThemeToControl(CWnd* pControl);
    void ApplyDarkTitleBar(CWnd* pWnd);
    void ApplyThemeToPropertySheet(CPropertySheet* pSheet);
    void UpdateAllControls(CWnd* pWnd);  // Update all child controls recursively
    
    // Gradient and visual effects
    void DrawGradientBackground(CDC* pDC, CRect rect);
    void DrawControlBorder(CDC* pDC, CRect rect);
    void DrawThemedButton(CDC* pDC, CRect rect, LPCTSTR text, UINT state);
    void DrawThemedTab(CDC* pDC, CRect rect, LPCTSTR text, bool selected, bool hot);
    
    // Persistence
    void LoadThemePreference();
    void SaveThemePreference();
    
    // System theme detection (Windows 10+)
    bool IsSystemDarkMode() const;
    
    // Brushes for WM_CTLCOLOR
    CBrush* GetBackgroundBrush();
    CBrush* GetControlBackgroundBrush();
    CBrush* GetDialogBackgroundBrush() { return GetBackgroundBrush(); }  // Alias for consistency
    
private:
    ThemeManager();
    ~ThemeManager();
    
    // Prevent copying
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;
    
    AppTheme m_currentTheme;
    
    // Color schemes
    struct ColorScheme {
    	COLORREF background;
    	COLORREF text;
    	COLORREF controlBackground;
    	COLORREF border;
    	COLORREF highlight;
    	COLORREF disabledText;
    	COLORREF buttonBackground;      // Button background color
    	COLORREF buttonBorder;          // Button border color
    	COLORREF gradientStart;         // Gradient start color
    	COLORREF gradientEnd;           // Gradient end color
    };
    
    ColorScheme m_lightColors;
    ColorScheme m_darkColors;
    
    // Brushes for painting
    CBrush m_backgroundBrush;
    CBrush m_controlBackgroundBrush;
    
    const ColorScheme& GetCurrentColors() const;
    void UpdateBrushes();
};

#endif // THEME_MANAGER_H
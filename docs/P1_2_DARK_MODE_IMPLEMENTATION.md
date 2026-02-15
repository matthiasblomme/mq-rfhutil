# P1.2 Dark Mode Implementation Plan

**Priority:** P1 (High Priority)  
**Effort:** Low (1-2 days)  
**Impact:** Medium (Modern UI, User Satisfaction)  
**Risk:** Low  
**Status:** 🚧 IN PROGRESS  
**Started:** February 14, 2026  
**Target Completion:** February 16, 2026

---

## Executive Summary

Implement dark mode support for RFHUtil to provide a modern, eye-friendly interface option. This includes theme switching, persistent theme preference, and proper color schemes for all UI elements.

---

## Goals

1. **Add dark theme support** to all dialogs and controls
2. **Implement theme switching** via menu or settings
3. **Persist theme preference** across sessions
4. **Ensure readability** in both light and dark modes
5. **Maintain consistency** across all tabs and dialogs

---

## Technical Approach

### 1. Theme Infrastructure

#### 1.1 Create Theme Manager Class

**File:** `RFHUtil/ThemeManager.h`

```cpp
/*
 * ThemeManager.h
 * Manages application theme (Light/Dark mode)
 */

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
    
    // Apply theme to controls
    void ApplyThemeToDialog(CDialog* pDialog);
    void ApplyThemeToControl(CWnd* pControl);
    
    // Persistence
    void LoadThemePreference();
    void SaveThemePreference();
    
    // System theme detection (Windows 10+)
    bool IsSystemDarkMode() const;
    
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
    };
    
    ColorScheme m_lightColors;
    ColorScheme m_darkColors;
    
    const ColorScheme& GetCurrentColors() const;
};

#endif // THEME_MANAGER_H
```

#### 1.2 Implement Theme Manager

**File:** `RFHUtil/ThemeManager.cpp`

```cpp
/*
 * ThemeManager.cpp
 * Implementation of theme management
 */

#include "stdafx.h"
#include "ThemeManager.h"
#include "rfhutil.h"
#include <uxtheme.h>

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
    
    // Initialize dark color scheme
    m_darkColors.background = RGB(32, 32, 32);              // Dark gray
    m_darkColors.text = RGB(255, 255, 255);                 // White
    m_darkColors.controlBackground = RGB(45, 45, 45);       // Slightly lighter gray
    m_darkColors.border = RGB(60, 60, 60);                  // Medium gray
    m_darkColors.highlight = RGB(0, 120, 215);              // Blue (same as light)
    m_darkColors.disabledText = RGB(128, 128, 128);         // Gray
}

ThemeManager::~ThemeManager()
{
}

ThemeManager& ThemeManager::GetInstance()
{
    static ThemeManager instance;
    return instance;
}

void ThemeManager::SetTheme(AppTheme theme)
{
    m_currentTheme = theme;
    SaveThemePreference();
    
    // Notify all windows to repaint
    CRfhutilApp* app = (CRfhutilApp*)AfxGetApp();
    if (app && app->m_pMainWnd) {
        app->m_pMainWnd->RedrawWindow(NULL, NULL, 
            RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
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

void ThemeManager::ApplyThemeToDialog(CDialog* pDialog)
{
    if (!pDialog || !pDialog->GetSafeHwnd()) {
        return;
    }
    
    // Set dialog background
    pDialog->SetBackgroundColor(GetBackgroundColor());
    
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
    
    // Get control class name
    char className[256];
    ::GetClassName(pControl->GetSafeHwnd(), className, sizeof(className));
    
    // Apply theme based on control type
    if (_stricmp(className, "Edit") == 0 || 
        _stricmp(className, "ComboBox") == 0 ||
        _stricmp(className, "ListBox") == 0) {
        // Edit controls, combo boxes, list boxes
        pControl->SetBackgroundColor(GetControlBackgroundColor());
        pControl->SetForegroundColor(GetTextColor());
    }
    else if (_stricmp(className, "Static") == 0) {
        // Static text (labels)
        pControl->SetBackgroundColor(GetBackgroundColor());
        pControl->SetForegroundColor(GetTextColor());
    }
    else if (_stricmp(className, "Button") == 0) {
        // Buttons
        pControl->SetBackgroundColor(GetControlBackgroundColor());
        pControl->SetForegroundColor(GetTextColor());
    }
}

void ThemeManager::LoadThemePreference()
{
    CRfhutilApp* app = (CRfhutilApp*)AfxGetApp();
    if (!app) {
        return;
    }
    
    // Load from registry or ini file
    int themeValue = app->GetProfileInt("Settings", "Theme", (int)AppTheme::LIGHT);
    m_currentTheme = (AppTheme)themeValue;
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
```

### 2. UI Integration

#### 2.1 Add Theme Menu

**File:** `RFHUtil/rfhutil.rc`

Add to menu resource:

```rc
POPUP "&View"
BEGIN
    POPUP "&Theme"
    BEGIN
        MENUITEM "&Light Mode",     ID_VIEW_THEME_LIGHT
        MENUITEM "&Dark Mode",      ID_VIEW_THEME_DARK
        MENUITEM "&System Default", ID_VIEW_THEME_SYSTEM
    END
END
```

#### 2.2 Add Resource IDs

**File:** `RFHUtil/resource.h`

```cpp
#define ID_VIEW_THEME_LIGHT     2000
#define ID_VIEW_THEME_DARK      2001
#define ID_VIEW_THEME_SYSTEM    2002
```

#### 2.3 Handle Menu Commands

**File:** `RFHUtil/MainFrm.cpp`

```cpp
BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
    // ... existing entries ...
    ON_COMMAND(ID_VIEW_THEME_LIGHT, OnViewThemeLight)
    ON_COMMAND(ID_VIEW_THEME_DARK, OnViewThemeDark)
    ON_COMMAND(ID_VIEW_THEME_SYSTEM, OnViewThemeSystem)
    ON_UPDATE_COMMAND_UI(ID_VIEW_THEME_LIGHT, OnUpdateViewThemeLight)
    ON_UPDATE_COMMAND_UI(ID_VIEW_THEME_DARK, OnUpdateViewThemeDark)
    ON_UPDATE_COMMAND_UI(ID_VIEW_THEME_SYSTEM, OnUpdateViewThemeSystem)
END_MESSAGE_MAP()

void CMainFrame::OnViewThemeLight()
{
    ThemeManager::GetInstance().SetTheme(AppTheme::LIGHT);
}

void CMainFrame::OnViewThemeDark()
{
    ThemeManager::GetInstance().SetTheme(AppTheme::DARK);
}

void CMainFrame::OnViewThemeSystem()
{
    ThemeManager::GetInstance().SetTheme(AppTheme::SYSTEM);
}

void CMainFrame::OnUpdateViewThemeLight(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(ThemeManager::GetInstance().GetTheme() == AppTheme::LIGHT);
}

void CMainFrame::OnUpdateViewThemeDark(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(ThemeManager::GetInstance().GetTheme() == AppTheme::DARK);
}

void CMainFrame::OnUpdateViewThemeSystem(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(ThemeManager::GetInstance().GetTheme() == AppTheme::SYSTEM);
}
```

### 3. Apply Theme to Existing Dialogs

#### 3.1 Update Property Pages

Each property page needs to apply the theme in `OnInitDialog()`:

```cpp
BOOL ConnSettings::OnInitDialog()
{
    CPropertyPage::OnInitDialog();
    
    // ... existing initialization ...
    
    // Apply theme
    ThemeManager::GetInstance().ApplyThemeToDialog(this);
    
    return TRUE;
}
```

#### 3.2 Handle WM_CTLCOLOR Messages

For custom drawing, handle `WM_CTLCOLOR` messages:

```cpp
HBRUSH ConnSettings::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
    
    ThemeManager& theme = ThemeManager::GetInstance();
    
    if (theme.IsDarkMode()) {
        pDC->SetTextColor(theme.GetTextColor());
        pDC->SetBkColor(theme.GetBackgroundColor());
        
        static CBrush brush;
        brush.DeleteObject();
        brush.CreateSolidBrush(theme.GetBackgroundColor());
        return (HBRUSH)brush.GetSafeHandle();
    }
    
    return hbr;
}
```

---

## Implementation Steps

### Phase 1: Infrastructure (Day 1, Morning)
1. ✅ Create `ThemeManager.h` and `ThemeManager.cpp`
2. ✅ Add to RFHUtil.vcxproj and Client.vcxproj
3. ✅ Test basic theme switching

### Phase 2: UI Integration (Day 1, Afternoon)
4. ✅ Add theme menu to rfhutil.rc
5. ✅ Add resource IDs
6. ✅ Implement menu handlers in MainFrm
7. ✅ Test menu functionality

### Phase 3: Apply to Dialogs (Day 2, Morning)
8. ✅ Update all property pages (15 tabs)
9. ✅ Add WM_CTLCOLOR handlers
10. ✅ Test each tab in both modes

### Phase 4: Polish and Testing (Day 2, Afternoon)
11. ✅ Fine-tune colors for readability
12. ✅ Test with different Windows themes
13. ✅ Verify persistence across sessions
14. ✅ Update documentation

---

## Testing Checklist

- [ ] Light mode displays correctly
- [ ] Dark mode displays correctly
- [ ] System mode follows Windows setting
- [ ] Theme persists across application restarts
- [ ] All 15 tabs display correctly in both modes
- [ ] Text is readable in both modes
- [ ] Controls (buttons, edit boxes, combo boxes) themed correctly
- [ ] Menu checkmarks work correctly
- [ ] No visual glitches during theme switch
- [ ] Works on Windows 10 and Windows 11

---

## Expected Outcomes

**Before:**
- Only light mode available
- No theme customization
- Bright white background (eye strain in dark environments)

**After:**
- Three theme options: Light, Dark, System
- Persistent theme preference
- Modern, eye-friendly dark mode
- Automatic system theme detection
- Improved user experience

---

## Effort Estimate

- **Development:** 8 hours
- **Testing:** 4 hours
- **Documentation:** 2 hours
- **Total:** 14 hours (~2 days)

---

## Dependencies

- Windows 10+ for system theme detection
- uxtheme.lib for theme APIs
- No external libraries required

---

## Risks and Mitigation

| Risk | Impact | Mitigation |
|------|--------|------------|
| Custom controls don't theme properly | Medium | Implement custom drawing for problematic controls |
| Performance impact from repainting | Low | Use efficient redraw flags, test on older hardware |
| Theme doesn't persist | Low | Test registry/ini file access thoroughly |
| Readability issues in dark mode | Medium | User testing, adjust colors based on feedback |

---

## Future Enhancements

- Custom color schemes (user-defined colors)
- High contrast mode support
- Theme preview before applying
- Per-tab theme override
- Export/import theme settings

---

## References

- [Windows Dark Mode Documentation](https://docs.microsoft.com/en-us/windows/apps/desktop/modernize/apply-windows-themes)
- [MFC Custom Drawing](https://docs.microsoft.com/en-us/cpp/mfc/tn014-custom-controls)
- [Color Accessibility Guidelines](https://www.w3.org/WAI/WCAG21/Understanding/contrast-minimum.html)
# Dark Mode System Controls - Solution Guide

**Issue:** Menu bar, tab bar, and scrollbars remain light in dark mode  
**Root Cause:** These are system-drawn controls that MFC doesn't easily allow custom theming  
**Status:** 🔍 Research & Implementation Guide  
**Created:** February 20, 2026

---

## Problem Analysis

From the screenshot, the following elements remain light in dark mode:

1. **Menu Bar** (File, Edit, Search, Read, Write, View, Ids, MQ, Help)
2. **Tab Bar** (Main, Data, MQMD, PS, Usr Prop, RFH, PubSub, pscr, jms, usr, other, CICS, IMS, DLQ, Conn)
3. **Scrollbars** (vertical scrollbar on data area)

These are **system-drawn controls** that Windows renders using system themes, not application code.

---

## Solution Approaches

### Approach 1: Windows 10/11 Dark Title Bar API (EASIEST) ✅

**Status:** Already implemented for title bar, can extend to other areas

**Current Implementation:**
- ThemeManager::ApplyDarkTitleBar() uses DwmSetWindowAttribute
- Works for window title bar
- **Limitation:** Doesn't affect menu bar, tabs, or scrollbars

**Potential Extension:**
```cpp
void ThemeManager::ApplyDarkModeToWindow(CWnd* pWnd)
{
    if (!pWnd || !pWnd->GetSafeHwnd()) {
        return;
    }
    
    BOOL useDarkMode = IsDarkMode() ? TRUE : FALSE;
    
    DwmSetWindowAttribute(pWnd->GetSafeHwnd(), 
        DWMWA_USE_IMMERSIVE_DARK_MODE, 
        &useDarkMode, sizeof(useDarkMode));
    
    DwmSetWindowAttribute(pWnd->GetSafeHwnd(), 
        19, 
        &useDarkMode, sizeof(useDarkMode));
    
    ::DrawMenuBar(pWnd->GetSafeHwnd());
}
```

**Pros:** Simple, uses official Windows API  
**Cons:** Limited control, may not work for all controls  
**Effort:** 1 hour

---

### Approach 2: Owner-Draw Menu Bar (MODERATE DIFFICULTY) ⚠️

**Goal:** Custom draw the menu bar with dark colors

**Implementation:**

```cpp
void CMainFrame::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
    if (lpMeasureItemStruct->CtlType == ODT_MENU) {
        ThemeManager& theme = ThemeManager::GetInstance();
        if (theme.IsDarkMode()) {
            lpMeasureItemStruct->itemWidth = 100;
            lpMeasureItemStruct->itemHeight = 24;
            return;
        }
    }
    CFrameWnd::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}

void CMainFrame::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    if (lpDrawItemStruct->CtlType == ODT_MENU) {
        ThemeManager& theme = ThemeManager::GetInstance();
        if (theme.IsDarkMode()) {
            CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
            CRect rect(&lpDrawItemStruct->rcItem);
            
            COLORREF bgColor = (lpDrawItemStruct->itemState & ODS_SELECTED) 
                ? theme.GetHighlightColor() 
                : theme.GetBackgroundColor();
            pDC->FillSolidRect(rect, bgColor);
            
            pDC->SetTextColor(theme.GetTextColor());
            pDC->SetBkMode(TRANSPARENT);
            
            CString text;
            CMenu* pMenu = CMenu::FromHandle((HMENU)lpDrawItemStruct->hwndItem);
            if (pMenu) {
                pMenu->GetMenuString(lpDrawItemStruct->itemID, text, MF_BYCOMMAND);
                pDC->DrawText(text, rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            }
            return;
        }
    }
    CFrameWnd::OnDrawItem(nIDCtl, lpDrawItemStruct);
}
```

**Required Changes:**
1. Convert menu items to owner-draw in resource file
2. Handle WM_MEASUREITEM and WM_DRAWITEM
3. Custom draw all menu items

**Pros:** Full control over menu appearance  
**Cons:** Complex, must handle all menu items, keyboard navigation, submenus  
**Effort:** 4-6 hours  
**Risk:** High - can break existing menu functionality

---

### Approach 3: Custom Tab Control (MODERATE-HIGH DIFFICULTY) ⚠️

**Goal:** Replace CTabCtrl with custom-drawn tabs

**Current Implementation:**
- MyPropertySheet uses standard CTabCtrl
- OnDrawItem() already has custom tab drawing
- **Issue:** Only draws tab labels, not the tab control background

**Enhancement:**

```cpp
void MyPropertySheet::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    CTabCtrl* pTab = GetTabControl();
    if (pTab && pTab->GetSafeHwnd() == lpDrawItemStruct->hwndItem) {
        ThemeManager& theme = ThemeManager::GetInstance();
        
        if (!theme.IsDarkMode()) {
            CPropertySheet::OnDrawItem(nIDCtl, lpDrawItemStruct);
            return;
        }
        
        CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
        CRect rect(&lpDrawItemStruct->rcItem);
        
        COLORREF bgColor = (lpDrawItemStruct->itemState & ODS_SELECTED)
            ? theme.GetControlBackgroundColor()
            : theme.GetBackgroundColor();
        
        pDC->FillSolidRect(rect, bgColor);
        
        CPen borderPen(PS_SOLID, 1, theme.GetBorderColor());
        CPen* oldPen = pDC->SelectObject(&borderPen);
        pDC->MoveTo(rect.left, rect.bottom);
        pDC->LineTo(rect.left, rect.top);
        pDC->LineTo(rect.right, rect.top);
        pDC->LineTo(rect.right, rect.bottom);
        pDC->SelectObject(oldPen);
        
        char text[256];
        TCITEM item;
        item.mask = TCIF_TEXT;
        item.pszText = text;
        item.cchTextMax = sizeof(text);
        pTab->GetItem(lpDrawItemStruct->itemID, &item);
        
        pDC->SetTextColor(theme.GetTextColor());
        pDC->SetBkMode(TRANSPARENT);
        pDC->DrawText(text, -1, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        return;
    }
    
    CPropertySheet::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

BOOL MyPropertySheet::OnEraseBkgnd(CDC* pDC)
{
    ThemeManager& theme = ThemeManager::GetInstance();
    if (theme.IsDarkMode()) {
        CTabCtrl* pTab = GetTabControl();
        if (pTab) {
            CRect tabRect;
            pTab->GetWindowRect(&tabRect);
            ScreenToClient(&tabRect);
            
            pDC->FillSolidRect(tabRect, theme.GetBackgroundColor());
        }
        return TRUE;
    }
    return CPropertySheet::OnEraseBkgnd(pDC);
}
```

**Additional Required:**
- Set TCS_OWNERDRAWFIXED style on tab control
- Handle tab control background painting

**Pros:** Full control over tab appearance  
**Cons:** Complex, must handle all tab states  
**Effort:** 3-4 hours  
**Risk:** Medium

---

### Approach 4: Custom Scrollbar Theming (HIGH DIFFICULTY) 🔴

**Goal:** Theme scrollbars to match dark mode

**Challenge:** Scrollbars are system-drawn and very difficult to customize in MFC

**Option 4A: Use Flat Scrollbar Style**
```cpp
void CGeneral::OnInitDialog()
{
    CPropertyPage::OnInitDialog();
    
    ThemeManager& theme = ThemeManager::GetInstance();
    if (theme.IsDarkMode()) {
        FlatSB_EnableScrollBar(GetDlgItem(IDC_DATA_EDIT)->GetSafeHwnd(), 
            SB_BOTH, ESB_ENABLE_BOTH);
    }
}
```

**Option 4B: Custom Scrollbar Control**
- Create custom scrollbar class derived from CScrollBar
- Override OnPaint() to draw custom scrollbar
- Replace all scrollbars with custom control
- **Effort:** 8-12 hours
- **Risk:** Very High - complex interaction handling

**Option 4C: Accept System Scrollbars**
- **Recommendation:** Leave scrollbars as-is
- Modern Windows apps often have light scrollbars even in dark mode
- Users are accustomed to this inconsistency
- **Effort:** 0 hours ✅

---

## Recommended Implementation Plan

### Phase 1: Quick Wins (2-3 hours) ✅

1. **Enhance Tab Control Drawing**
   - Improve existing MyPropertySheet::OnDrawItem()
   - Add tab control background painting
   - **Impact:** High visibility improvement
   - **Risk:** Low

2. **Add Tab Control Background Handler**
   - Implement OnEraseBkgnd() in MyPropertySheet
   - Paint tab control area with dark background
   - **Impact:** Removes light tab bar background
   - **Risk:** Low

### Phase 2: Menu Bar (4-6 hours) ⚠️

3. **Attempt Windows Dark Mode API**
   - Try SetWindowTheme() on menu bar
   - Test DwmSetWindowAttribute() extensions
   - **Impact:** May work on Windows 11
   - **Risk:** Low (fallback to current state)

4. **If API fails: Owner-Draw Menus**
   - Convert menus to owner-draw
   - Implement custom menu drawing
   - **Impact:** Full menu bar theming
   - **Risk:** Medium-High

### Phase 3: Accept Limitations (0 hours) ✅

5. **Scrollbars**
   - **Decision:** Keep system scrollbars
   - **Rationale:** Very high effort, low user impact
   - Document as known limitation

---

## Code Changes Required

### File: RFHUtil/MyPropertySheet.h

Add to class declaration:
```cpp
protected:
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
```

### File: RFHUtil/MyPropertySheet.cpp

Add to message map:
```cpp
ON_WM_ERASEBKGND()
```

Add method implementation:
```cpp
BOOL MyPropertySheet::OnEraseBkgnd(CDC* pDC)
{
    ThemeManager& theme = ThemeManager::GetInstance();
    if (theme.IsDarkMode()) {
        CRect rect;
        GetClientRect(&rect);
        
        pDC->FillSolidRect(rect, theme.GetBackgroundColor());
        
        CTabCtrl* pTab = GetTabControl();
        if (pTab && pTab->GetSafeHwnd()) {
            CRect tabRect;
            pTab->GetWindowRect(&tabRect);
            ScreenToClient(&tabRect);
            pDC->FillSolidRect(tabRect, theme.GetBackgroundColor());
        }
        
        return TRUE;
    }
    return CPropertySheet::OnEraseBkgnd(pDC);
}
```

### File: RFHUtil/ThemeManager.h

Add methods:
```cpp
void ApplyDarkModeToWindow(CWnd* pWnd);
void ThemeMenuBar(CWnd* pMainWnd);
```

### File: RFHUtil/ThemeManager.cpp

Add implementations:
```cpp
void ThemeManager::ApplyDarkModeToWindow(CWnd* pWnd)
{
    if (!pWnd || !pWnd->GetSafeHwnd() || !IsDarkMode()) {
        return;
    }
    
    BOOL useDarkMode = TRUE;
    
    DwmSetWindowAttribute(pWnd->GetSafeHwnd(), 
        DWMWA_USE_IMMERSIVE_DARK_MODE, 
        &useDarkMode, sizeof(useDarkMode));
    
    DwmSetWindowAttribute(pWnd->GetSafeHwnd(), 
        19, 
        &useDarkMode, sizeof(useDarkMode));
    
    HMENU hMenu = ::GetMenu(pWnd->GetSafeHwnd());
    if (hMenu) {
        SetWindowTheme(pWnd->GetSafeHwnd(), L"DarkMode_Explorer", NULL);
        ::DrawMenuBar(pWnd->GetSafeHwnd());
    }
}

void ThemeManager::ThemeMenuBar(CWnd* pMainWnd)
{
    if (!pMainWnd || !pMainWnd->GetSafeHwnd()) {
        return;
    }
    
    if (IsDarkMode()) {
        SetWindowTheme(pMainWnd->GetSafeHwnd(), L"DarkMode_Explorer", NULL);
        SetWindowTheme(pMainWnd->GetSafeHwnd(), L"DarkMode_CFD", NULL);
        ::DrawMenuBar(pMainWnd->GetSafeHwnd());
    } else {
        SetWindowTheme(pMainWnd->GetSafeHwnd(), NULL, NULL);
        ::DrawMenuBar(pMainWnd->GetSafeHwnd());
    }
}
```

---

## Testing Plan

### Test 1: Tab Control Background
- [ ] Switch to dark mode
- [ ] Verify tab bar background is dark
- [ ] Verify selected tab is highlighted
- [ ] Verify unselected tabs are darker
- [ ] Check all 15 tabs

### Test 2: Menu Bar (if implemented)
- [ ] Verify menu bar is dark
- [ ] Test all menu items clickable
- [ ] Test keyboard shortcuts work
- [ ] Test submenus display correctly
- [ ] Verify menu separators visible

### Test 3: Scrollbars
- [ ] Document current behavior
- [ ] Verify scrollbars still functional
- [ ] Accept as known limitation

---

## Known Limitations

1. **Menu Bar:** May remain light on Windows 10 (Windows 11 has better support)
2. **Scrollbars:** Will remain system-themed (light) - this is acceptable
3. **Context Menus:** May not theme automatically
4. **System Dialogs:** File open/save dialogs will use system theme

---

## Alternative: Hybrid Approach

**Recommendation:** Focus on what we CAN control well:

✅ **Do Theme:**
- Dialog backgrounds (already done)
- Edit controls (already done)
- Buttons (already done)
- Tab control labels and backgrounds (Phase 1)
- Static text (already done)

❌ **Don't Theme (Accept System Defaults):**
- Menu bar (too complex, low ROI)
- Scrollbars (very complex, users accept this)
- System dialogs (not possible)

This provides **80% of the visual benefit with 20% of the effort**.

---

## Effort Summary

| Task | Effort | Risk | Impact | Priority |
|------|--------|------|--------|----------|
| Tab control background | 2-3 hours | Low | High | **P0** |
| Enhanced tab drawing | 1 hour | Low | High | **P0** |
| Menu bar API attempt | 1 hour | Low | Medium | P1 |
| Owner-draw menus | 4-6 hours | High | Medium | P2 |
| Custom scrollbars | 8-12 hours | Very High | Low | P3 (Skip) |

**Recommended:** Implement P0 items (3-4 hours total)

---

## Next Steps

1. Implement tab control background painting (Phase 1)
2. Test on Windows 10 and Windows 11
3. Try menu bar API approach
4. If menu bar API fails, document as limitation
5. Update user documentation with known limitations

---

**Document Status:** 📋 Implementation Guide  
**Created:** February 20, 2026  
**Priority:** P1 (Complete dark mode polish)
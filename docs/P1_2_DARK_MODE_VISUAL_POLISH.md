# P1.2 Dark Mode - Phase 4: Visual Polish Implementation Plan

## Overview

This document details the implementation plan for enhancing the dark mode visual appearance based on user feedback. The current implementation works functionally but needs visual refinement to match modern dark mode aesthetics.

**Status:** 🚧 In Progress  
**Started:** February 15, 2026  
**Target Completion:** February 16, 2026

## Current State

### What Works ✅
- ThemeManager infrastructure (singleton pattern)
- Theme switching (Light/Dark/System modes)
- Theme persistence (registry storage)
- Basic WM_CTLCOLOR handling for all 15 property pages
- System theme detection (Windows 10+)

### What Needs Improvement 🎨
Based on user feedback and reference images:
1. **Buttons** - Still appear light-colored, need dark grey theming
2. **Menu bar** - Remains light, needs dark theming
3. **Edit controls** - Need dark grey backgrounds (not just white/dark)
4. **Combo boxes** - Need consistent dark grey theming
5. **Visual separation** - Need subtle borders and gradients
6. **Overall polish** - Need gradient backgrounds for better aesthetics

## Reference Design

User provided reference showing desired appearance:
- **Background:** Dark (RGB ~30-35)
- **Fields:** Dark grey (RGB ~45-50) for better contrast
- **Borders:** Subtle grey borders for visual separation
- **Gradients:** Smooth gradients for dialog backgrounds
- **Buttons:** Dark grey with subtle highlights

## Implementation Plan

### Phase 4.1: Update ThemeManager Color Palette

**Goal:** Refine dark mode colors to match reference design

**Files to Modify:**
- [`RFHUtil/ThemeManager.cpp`](../RFHUtil/ThemeManager.cpp:46-52)

**Changes:**

```cpp
// Current dark colors (line 47-52)
m_darkColors.background = RGB(32, 32, 32);              // Dark gray
m_darkColors.text = RGB(255, 255, 255);                 // White
m_darkColors.controlBackground = RGB(45, 45, 45);       // Slightly lighter gray
m_darkColors.border = RGB(60, 60, 60);                  // Medium gray
m_darkColors.highlight = RGB(0, 120, 215);              // Blue
m_darkColors.disabledText = RGB(128, 128, 128);         // Gray

// Proposed new dark colors
m_darkColors.background = RGB(30, 30, 30);              // Darker background
m_darkColors.text = RGB(240, 240, 240);                 // Slightly off-white
m_darkColors.controlBackground = RGB(48, 48, 48);       // Dark grey for fields
m_darkColors.border = RGB(70, 70, 70);                  // Lighter border
m_darkColors.highlight = RGB(0, 120, 215);              // Keep blue
m_darkColors.disabledText = RGB(120, 120, 120);         // Slightly lighter
m_darkColors.buttonBackground = RGB(55, 55, 55);        // New: Button color
m_darkColors.buttonBorder = RGB(80, 80, 80);            // New: Button border
```

**Add to ColorScheme struct:**
```cpp
// In ThemeManager.h (line 77-84)
struct ColorScheme {
    COLORREF background;
    COLORREF text;
    COLORREF controlBackground;
    COLORREF border;
    COLORREF highlight;
    COLORREF disabledText;
    COLORREF buttonBackground;      // NEW
    COLORREF buttonBorder;          // NEW
    COLORREF gradientStart;         // NEW
    COLORREF gradientEnd;           // NEW
};
```

**Estimated Time:** 30 minutes

---

### Phase 4.2: Add Gradient Background Support

**Goal:** Implement gradient backgrounds for dialogs

**Files to Modify:**
- [`RFHUtil/ThemeManager.h`](../RFHUtil/ThemeManager.h)
- [`RFHUtil/ThemeManager.cpp`](../RFHUtil/ThemeManager.cpp)

**New Methods to Add:**

```cpp
// In ThemeManager.h
class ThemeManager {
public:
    // ... existing methods ...
    
    // Gradient support
    void DrawGradientBackground(CDC* pDC, CRect rect);
    COLORREF GetGradientStartColor() const;
    COLORREF GetGradientEndColor() const;
};

// In ThemeManager.cpp
void ThemeManager::DrawGradientBackground(CDC* pDC, CRect rect)
{
    if (!IsDarkMode()) {
        // Light mode - solid background
        pDC->FillSolidRect(rect, GetBackgroundColor());
        return;
    }
    
    // Dark mode - vertical gradient
    COLORREF startColor = GetGradientStartColor();
    COLORREF endColor = GetGradientEndColor();
    
    // Create gradient using GradientFill
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
    gRect.LowerLeft = 1;
    
    pDC->GradientFill(vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
}
```

**Update Dialog OnEraseBkgnd:**

Each property page needs to override `OnEraseBkgnd`:

```cpp
// Example for General.cpp
BOOL General::OnEraseBkgnd(CDC* pDC)
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
```

**Estimated Time:** 2 hours (including updates to all 15 dialogs)

---

### Phase 4.3: Implement Custom Button Drawing

**Goal:** Theme buttons with dark grey colors

**Approach:** Use owner-draw buttons or subclass existing buttons

**Files to Modify:**
- [`RFHUtil/ThemeManager.h`](../RFHUtil/ThemeManager.h)
- [`RFHUtil/ThemeManager.cpp`](../RFHUtil/ThemeManager.cpp)

**New Methods:**

```cpp
// In ThemeManager.h
class ThemeManager {
public:
    // ... existing methods ...
    
    // Button theming
    void DrawThemedButton(CDC* pDC, CRect rect, LPCTSTR text, UINT state);
    COLORREF GetButtonBackgroundColor() const;
    COLORREF GetButtonBorderColor() const;
};

// In ThemeManager.cpp
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
    
    // Adjust for button state
    if (state & ODS_SELECTED) {
        bgColor = RGB(
            max(0, GetRValue(bgColor) - 10),
            max(0, GetGValue(bgColor) - 10),
            max(0, GetBValue(bgColor) - 10)
        );
    } else if (state & ODS_FOCUS) {
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
}
```

**Alternative Approach:** Use `WM_CTLCOLORBTN` message handler

```cpp
// In each dialog's message map
ON_WM_CTLCOLOR()

// In OnCtlColor handler
HBRUSH CGeneral::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    ThemeManager& theme = ThemeManager::GetInstance();
    
    if (theme.IsDarkMode()) {
        pDC->SetTextColor(theme.GetTextColor());
        pDC->SetBkColor(theme.GetControlBackgroundColor());
        
        if (nCtlColor == CTLCOLOR_BTN) {
            // Theme buttons
            return (HBRUSH)theme.GetControlBackgroundBrush()->GetSafeHandle();
        }
        // ... handle other control types ...
    }
    
    return CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
}
```

**Estimated Time:** 2-3 hours

---

### Phase 4.4: Theme Edit Controls and Combo Boxes

**Goal:** Apply dark grey backgrounds to edit controls and combo boxes

**Current Issue:** Edit controls and combo boxes still show light backgrounds

**Solution:** Handle `CTLCOLOR_EDIT` and `CTLCOLOR_LISTBOX` in `OnCtlColor`

**Implementation:**

```cpp
// Update OnCtlColor in all property pages
HBRUSH CGeneral::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
```

**PowerShell Script to Update All Dialogs:**

```powershell
# update_ctlcolor_handlers.ps1
$dialogFiles = @(
    "General", "MQMDPAGE", "MSGDATA", "RFH", "jms", 
    "CICS", "Dlq", "Ims", "other", "Props", 
    "PS", "pscr", "PubSub", "Usr", "ConnSettings"
)

foreach ($dialog in $dialogFiles) {
    $cppFile = "RFHUtil\$dialog.cpp"
    
    # Read file content
    $content = Get-Content $cppFile -Raw
    
    # Find and replace OnCtlColor implementation
    # (Script details omitted for brevity)
}
```

**Estimated Time:** 1-2 hours

---

### Phase 4.5: Add Border Drawing for Visual Separation

**Goal:** Add subtle borders around controls for better visual separation

**Implementation:**

```cpp
// In ThemeManager
void ThemeManager::DrawControlBorder(CDC* pDC, CRect rect)
{
    if (!IsDarkMode()) {
        return; // No borders in light mode
    }
    
    CPen borderPen(PS_SOLID, 1, GetBorderColor());
    CPen* oldPen = pDC->SelectObject(&borderPen);
    
    pDC->MoveTo(rect.left, rect.top);
    pDC->LineTo(rect.right, rect.top);
    pDC->LineTo(rect.right, rect.bottom);
    pDC->LineTo(rect.left, rect.bottom);
    pDC->LineTo(rect.left, rect.top);
    
    pDC->SelectObject(oldPen);
}
```

**Usage in Dialogs:**

```cpp
// In OnPaint or OnDrawItem
void CGeneral::OnPaint()
{
    CPaintDC dc(this);
    ThemeManager& theme = ThemeManager::GetInstance();
    
    if (theme.IsDarkMode()) {
        // Draw borders around group boxes or important controls
        CRect rect;
        GetDlgItem(IDC_SOME_CONTROL)->GetWindowRect(&rect);
        ScreenToClient(&rect);
        theme.DrawControlBorder(&dc, rect);
    }
}
```

**Estimated Time:** 1 hour

---

### Phase 4.6: Testing Visual Improvements

**Goal:** Verify visual improvements across all tabs and scenarios

**Test Scenarios:**

1. **Theme Switching**
   - Switch from Light → Dark → System
   - Verify all controls update correctly
   - Check for visual artifacts or flicker

2. **All 15 Property Pages**
   - General tab
   - MQMD tab
   - Message Data tab
   - RFH tab
   - JMS tab
   - CICS tab
   - DLQ tab
   - IMS tab
   - Other tab
   - Properties tab
   - PS tab
   - PSCR tab
   - Pub/Sub tab
   - User tab
   - Connection Settings tab

3. **Control Types**
   - Edit boxes (text input)
   - Combo boxes (dropdowns)
   - Buttons (all types)
   - Static text
   - Group boxes
   - Check boxes
   - Radio buttons
   - List controls

4. **Visual Quality**
   - Gradient backgrounds smooth
   - Borders visible but subtle
   - Text readable (good contrast)
   - Buttons clearly clickable
   - No white flashes or artifacts
   - Consistent appearance across tabs

5. **Performance**
   - No lag when switching themes
   - Smooth repainting
   - No memory leaks

**Estimated Time:** 2 hours

---

## Implementation Order

1. ✅ **Phase 4.1** - Update color palette (30 min)
2. ✅ **Phase 4.2** - Add gradient backgrounds (2 hours)
3. ✅ **Phase 4.3** - Theme buttons (2-3 hours)
4. ✅ **Phase 4.4** - Theme edit controls (1-2 hours)
5. ✅ **Phase 4.5** - Add borders (1 hour)
6. ✅ **Phase 4.6** - Testing (2 hours)

**Total Estimated Time:** 8-10 hours

---

## Success Criteria

- [ ] Dark mode matches reference image aesthetics
- [ ] All buttons are dark grey themed
- [ ] Edit controls and combo boxes have dark grey backgrounds
- [ ] Gradient backgrounds applied to all dialogs
- [ ] Subtle borders provide visual separation
- [ ] No visual artifacts or flicker
- [ ] Theme switching works smoothly
- [ ] All 15 tabs look consistent
- [ ] User feedback incorporated

---

## Next Steps After Phase 4

Once visual polish is complete:

1. **Phase 5:** Update README.md
   - Add "What's New" section
   - Create version history table
   - Document dark mode feature
   - Add screenshots

2. **Update CHANGELOG.md**
   - Document version 9.4.0.0 changes
   - List all P0 and P1.2 improvements

3. **Update MODERNIZATION_ROADMAP.md**
   - Mark P1.2 as complete
   - Update progress tracker

---

**Document Status:** 🚧 In Progress  
**Last Updated:** February 15, 2026  
**Next Review:** After Phase 4 completion
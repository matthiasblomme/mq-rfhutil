# Add OnDrawItem handlers to all property pages for button theming

$propertyPages = @(
    "General",
    "MQMDPAGE",
    "MSGDATA",
    "RFH",
    "jms",
    "CICS",
    "Dlq",
    "Ims",
    "other",
    "Props",
    "PS",
    "pscr",
    "PubSub",
    "Usr",
    "ConnSettings"
)

foreach ($page in $propertyPages) {
    $headerFile = "RFHUtil/$page.h"
    $cppFile = "RFHUtil/$page.cpp"
    
    Write-Host "Processing $page..."
    
    # Add declaration to header file
    if (Test-Path $headerFile) {
        $headerContent = Get-Content $headerFile -Raw
        
        # Check if OnDrawItem already exists
        if ($headerContent -notmatch "afx_msg void OnDrawItem") {
            # Find the OnEraseBkgnd line and add OnDrawItem after it
            $pattern = '(\s+afx_msg BOOL OnEraseBkgnd\(CDC\* pDC\);)'
            if ($headerContent -match $pattern) {
                $replacement = '$1' + "`r`n`tafx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);"
                $headerContent = $headerContent -replace $pattern, $replacement
                Set-Content -Path $headerFile -Value $headerContent -NoNewline
                Write-Host "  Added OnDrawItem declaration to $headerFile"
            } else {
                Write-Host "  [WARNING] Could not find OnEraseBkgnd in $headerFile"
            }
        } else {
            Write-Host "  OnDrawItem already exists in $headerFile"
        }
    }
    
    # Add message map entry to CPP file
    if (Test-Path $cppFile) {
        $cppContent = Get-Content $cppFile -Raw
        
        # Check if ON_WM_DRAWITEM already exists
        if ($cppContent -notmatch "ON_WM_DRAWITEM") {
            # Find ON_WM_ERASEBKGND and add ON_WM_DRAWITEM after it
            $pattern = '(\s+ON_WM_ERASEBKGND\(\))'
            if ($cppContent -match $pattern) {
                $replacement = '$1' + "`r`n`tON_WM_DRAWITEM()"
                $cppContent = $cppContent -replace $pattern, $replacement
                Set-Content -Path $cppFile -Value $cppContent -NoNewline
                Write-Host "  Added ON_WM_DRAWITEM to message map in $cppFile"
            } else {
                Write-Host "  [WARNING] Could not find ON_WM_ERASEBKGND in $cppFile"
            }
        } else {
            Write-Host "  ON_WM_DRAWITEM already exists in $cppFile"
        }
    }
}

Write-Host "`nPhase 1 complete: Added OnDrawItem declarations and message map entries"
Write-Host "Next: Run add_drawitem_implementations.ps1 to add the implementations"
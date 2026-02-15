# Fix all OnEraseBkgnd placement issues
# The OnEraseBkgnd was inserted in the middle of OnCtlColor functions

$fixes = @(
    @{File="RFHUtil/General.cpp"; StartLine=1262; EndLine=1306},
    @{File="RFHUtil/jms.cpp"; StartLine=1088; EndLine=1132},
    @{File="RFHUtil/MQMDPAGE.cpp"; StartLine=273; EndLine=317},
    @{File="RFHUtil/MSGDATA.cpp"; StartLine=1512; EndLine=1556},
    @{File="RFHUtil/other.cpp"; StartLine=872; EndLine=916},
    @{File="RFHUtil/pscr.cpp"; StartLine=1212; EndLine=1256},
    @{File="RFHUtil/PubSub.cpp"; StartLine=3925; EndLine=3969},
    @{File="RFHUtil/RFH.cpp"; StartLine=3719; EndLine=3763},
    @{File="RFHUtil/ThemeManager.cpp"; StartLine=324; EndLine=398},
    @{File="RFHUtil/Usr.cpp"; StartLine=823; EndLine=867}
)

foreach ($fix in $fixes) {
    $file = $fix.File
    Write-Host "Processing $file..."
    
    $lines = Get-Content $file
    $content = $lines -join "`r`n"
    
    # Generic pattern to fix the structure
    # Find: }BOOL ClassName::OnEraseBkgnd...}	}	return CPropertyPage::OnCtlColor...}
    # Replace with proper structure
    
    $pattern = '(\s+}\s*)\r?\nBOOL (\w+)::OnEraseBkgnd\(CDC\* pDC\)\r?\n\{\r?\n\s+ThemeManager& theme = ThemeManager::GetInstance\(\);\r?\n\s+\r?\n\s+if \(theme\.IsDarkMode\(\)\) \{\r?\n\s+CRect rect;\r?\n\s+GetClientRect\(&rect\);\r?\n\s+theme\.DrawGradientBackground\(pDC, rect\);\r?\n\s+return TRUE;\r?\n\s+}\r?\n\s+\r?\n\s+return CPropertyPage::OnEraseBkgnd\(pDC\);\r?\n}\r?\n(\s+}\s+\r?\n\s+return CPropertyPage::OnCtlColor\([^)]+\);\r?\n})'
    
    if ($content -match $pattern) {
        $className = $matches[2]
        # Reconstruct: close switch, close if, return, close function, then OnEraseBkgnd
        $replacement = '$1$3' + "`r`n`r`nBOOL $className::OnEraseBkgnd(CDC* pDC)`r`n{`r`n`tThemeManager& theme = ThemeManager::GetInstance();`r`n`t`r`n`tif (theme.IsDarkMode()) {`r`n`t`tCRect rect;`r`n`t`tGetClientRect(&rect);`r`n`t`ttheme.DrawGradientBackground(pDC, rect);`r`n`t`treturn TRUE;`r`n`t}`r`n`t`r`n`treturn CPropertyPage::OnEraseBkgnd(pDC);`r`n}"
        
        $newContent = $content -replace $pattern, $replacement
        Set-Content -Path $file -Value $newContent -NoNewline
        Write-Host "  Fixed $file"
    } else {
        Write-Host "  Pattern not matched for $file - needs manual fix"
    }
}

Write-Host "`nDone!"
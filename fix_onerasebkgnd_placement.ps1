# Fix OnEraseBkgnd functions that were inserted in the middle of OnCtlColor

$files = @(
    "RFHUtil/General.cpp",
    "RFHUtil/jms.cpp",
    "RFHUtil/MQMDPAGE.cpp",
    "RFHUtil/MSGDATA.cpp",
    "RFHUtil/other.cpp",
    "RFHUtil/pscr.cpp",
    "RFHUtil/PubSub.cpp",
    "RFHUtil/RFH.cpp",
    "RFHUtil/ThemeManager.cpp",
    "RFHUtil/Usr.cpp"
)

foreach ($file in $files) {
    Write-Host "Processing $file..."
    $content = Get-Content $file -Raw
    
    # Pattern: switch closing brace, then OnEraseBkgnd function, then the rest of OnCtlColor
    # We need to move OnEraseBkgnd after OnCtlColor completes
    $pattern = '(\s+}\s*)\r?\n(BOOL \w+::OnEraseBkgnd\(CDC\* pDC\)\s*\{[^}]+\{[^}]+\}[^}]+\})\r?\n(\s+}\s+return CPropertyPage::OnCtlColor[^}]+\})'
    
    if ($content -match $pattern) {
        # Reorder: switch close + OnCtlColor close, then OnEraseBkgnd
        $newContent = $content -replace $pattern, ('$1' + '$3' + "`r`n`r`n" + '$2')
        Set-Content -Path $file -Value $newContent -NoNewline
        Write-Host "  Fixed $file"
    } else {
        Write-Host "  Pattern not found in $file, checking manually..."
    }
}

Write-Host "Done!"
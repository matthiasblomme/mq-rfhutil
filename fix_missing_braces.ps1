# Fix missing closing braces before OnEraseBkgnd functions

$files = @(
    "RFHUtil/ConnSettings.cpp",
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
    
    # Pattern: return statement followed by OnEraseBkgnd without closing brace
    # Look for: "return ... ;\n\nBOOL ClassName::OnEraseBkgnd"
    $pattern = '(\r?\n\treturn [^;]+;)\r?\n(\r?\n)(BOOL \w+::OnEraseBkgnd)'
    $replacement = '$1' + "`r`n}`r`n`r`n" + '$3'
    
    $newContent = $content -replace $pattern, $replacement
    
    if ($content -ne $newContent) {
        Set-Content -Path $file -Value $newContent -NoNewline
        Write-Host "  Fixed $file"
    } else {
        Write-Host "  No changes needed for $file"
    }
}

Write-Host "Done!"
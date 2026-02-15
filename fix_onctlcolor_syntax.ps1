# PowerShell script to fix syntax errors in OnCtlColor implementations
# The issue is that the regex replacement removed the closing brace

$filesToFix = @(
    "General",
    "MQMDPAGE",
    "MSGDATA",
    "RFH",
    "jms",
    "other",
    "pscr",
    "PubSub",
    "Usr",
    "ConnSettings"
)

Write-Host "Fixing OnCtlColor syntax errors..." -ForegroundColor Cyan
Write-Host ""

foreach ($file in $filesToFix) {
    $cppFile = "RFHUtil\$file.cpp"
    
    if (-not (Test-Path $cppFile)) {
        Write-Host "  [SKIP] $file - File not found" -ForegroundColor Yellow
        continue
    }
    
    try {
        Write-Host "  [PROCESSING] $file..." -ForegroundColor White
        
        $content = Get-Content $cppFile -Raw
        
        # Find OnCtlColor implementation and check if it's missing closing brace
        if ($content -match "HBRUSH\s+$file\s*::\s*OnCtlColor\s*\([^)]+\)\s*\{[^}]*return\s+CPropertyPage::OnCtlColor\([^)]+\);(?!\s*\})") {
            # Add closing brace after the return statement
            $content = $content -replace "(return\s+CPropertyPage::OnCtlColor\([^)]+\);)(\s*)(BOOL|void|int|HBRUSH|\w+\s+$file)", "`$1`r`n}`r`n`r`n`$3"
            Set-Content -Path $cppFile -Value $content -NoNewline
            Write-Host "    [SUCCESS] Fixed missing closing brace" -ForegroundColor Green
        } else {
            Write-Host "    [SKIP] No syntax error found or already fixed" -ForegroundColor Yellow
        }
        
    } catch {
        Write-Host "    [ERROR] $_" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "Done! Please rebuild the project." -ForegroundColor Cyan
Write-Host ""
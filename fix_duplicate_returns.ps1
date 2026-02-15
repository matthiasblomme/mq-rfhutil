# PowerShell script to remove duplicate return statements left by the replacement

$filesToFix = @(
    @{File="MQMDPAGE"; Pattern="return CPropertyPage::OnCtlColor"},
    @{File="MSGDATA"; Pattern="return CPropertyPage::OnCtlColor"},
    @{File="RFH"; Pattern="return CPropertyPage::OnCtlColor"},
    @{File="jms"; Pattern="return CPropertyPage::OnCtlColor"},
    @{File="other"; Pattern="return CPropertyPage::OnCtlColor"},
    @{File="pscr"; Pattern="return CPropertyPage::OnCtlColor"},
    @{File="PubSub"; Pattern="return CPropertyPage::OnCtlColor"},
    @{File="Usr"; Pattern="return CPropertyPage::OnCtlColor"},
    @{File="ConnSettings"; Pattern="return CPropertyPage::OnCtlColor"}
)

Write-Host "Removing duplicate return statements..." -ForegroundColor Cyan
Write-Host ""

foreach ($item in $filesToFix) {
    $file = $item.File
    $cppFile = "RFHUtil\$file.cpp"
    
    if (-not (Test-Path $cppFile)) {
        Write-Host "  [SKIP] $file - File not found" -ForegroundColor Yellow
        continue
    }
    
    try {
        Write-Host "  [PROCESSING] $file..." -ForegroundColor White
        
        $content = Get-Content $cppFile -Raw
        
        # Pattern to find: return CPropertyPage::OnCtlColor(...); } [whitespace] return hbr; }
        # Replace with: return CPropertyPage::OnCtlColor(...); }
        $pattern = "(\treturn\s+CPropertyPage::OnCtlColor\([^)]+\);\r?\n\})\s+return\s+hbr;\r?\n\}"
        
        if ($content -match $pattern) {
            $content = $content -replace $pattern, '$1'
            Set-Content -Path $cppFile -Value $content -NoNewline
            Write-Host "    [SUCCESS] Removed duplicate return statement" -ForegroundColor Green
        } else {
            Write-Host "    [SKIP] No duplicate found" -ForegroundColor Yellow
        }
        
    } catch {
        Write-Host "    [ERROR] $_" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "Done!" -ForegroundColor Cyan
Write-Host ""
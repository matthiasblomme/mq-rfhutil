# Add OnEraseBkgnd to the remaining files that were skipped

$files = @("Dlq", "Ims", "Props", "PS")

$implementation = @'

BOOL CLASS_NAME::OnEraseBkgnd(CDC* pDC)
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
'@

Write-Host "Adding OnEraseBkgnd to remaining files..." -ForegroundColor Cyan

foreach ($file in $files) {
    $cppFile = "RFHUtil\$file.cpp"
    Write-Host "  Processing $file..." -ForegroundColor White
    
    if (Test-Path $cppFile) {
        $content = Get-Content $cppFile -Raw
        
        # Find OnCtlColor and add OnEraseBkgnd after it
        if ($content -match "HBRUSH\s+C?$file\s*::\s*OnCtlColor[^}]+return\s+CPropertyPage::OnCtlColor[^;]+;[\r\n]+\}") {
            $impl = $implementation -replace "CLASS_NAME", "C$file"
            $content = $content -replace "(HBRUSH\s+C?$file\s*::\s*OnCtlColor[^}]+return\s+CPropertyPage::OnCtlColor[^;]+;[\r\n]+\})", "`$1$impl"
            Set-Content -Path $cppFile -Value $content -NoNewline
            Write-Host "    [SUCCESS] Added OnEraseBkgnd" -ForegroundColor Green
        } else {
            Write-Host "    [ERROR] Could not find OnCtlColor" -ForegroundColor Red
        }
    }
}

Write-Host ""
Write-Host "Done!" -ForegroundColor Cyan
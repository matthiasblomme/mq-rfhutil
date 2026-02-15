# PowerShell script to update OnCtlColor handlers for enhanced dark mode theming
# This script updates all 15 property page dialogs to properly theme:
# - Edit controls (CTLCOLOR_EDIT)
# - List boxes/Combo boxes (CTLCOLOR_LISTBOX)
# - Buttons (CTLCOLOR_BTN)
# - Static text (CTLCOLOR_STATIC)
# - Dialog background (CTLCOLOR_DLG)

$dialogFiles = @(
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

$newOnCtlColorImplementation = @'
HBRUSH CLASS_NAME::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
'@

Write-Host "Updating OnCtlColor handlers for enhanced dark mode theming..." -ForegroundColor Cyan
Write-Host ""

$updatedCount = 0
$errorCount = 0

foreach ($dialog in $dialogFiles) {
    $cppFile = "RFHUtil\$dialog.cpp"
    
    if (-not (Test-Path $cppFile)) {
        Write-Host "  [SKIP] $dialog - File not found: $cppFile" -ForegroundColor Yellow
        continue
    }
    
    try {
        Write-Host "  [PROCESSING] $dialog..." -ForegroundColor White
        
        # Read file content
        $content = Get-Content $cppFile -Raw
        
        # Check if OnCtlColor exists
        if ($content -notmatch "HBRUSH\s+$dialog\s*::\s*OnCtlColor") {
            Write-Host "    [SKIP] No OnCtlColor method found" -ForegroundColor Yellow
            continue
        }
        
        # Find and replace the OnCtlColor implementation
        # Pattern matches the entire method from HBRUSH to the closing brace
        $pattern = "HBRUSH\s+$dialog\s*::\s*OnCtlColor\s*\([^)]+\)\s*\{[^}]*(?:\{[^}]*\}[^}]*)*\}"
        
        $replacement = $newOnCtlColorImplementation -replace "CLASS_NAME", $dialog
        
        if ($content -match $pattern) {
            $content = $content -replace $pattern, $replacement
            
            # Write back to file
            Set-Content -Path $cppFile -Value $content -NoNewline
            
            Write-Host "    [SUCCESS] Updated OnCtlColor handler" -ForegroundColor Green
            $updatedCount++
        } else {
            Write-Host "    [ERROR] Could not match OnCtlColor pattern" -ForegroundColor Red
            $errorCount++
        }
        
    } catch {
        Write-Host "    [ERROR] $_" -ForegroundColor Red
        $errorCount++
    }
}

Write-Host ""
Write-Host "Summary:" -ForegroundColor Cyan
Write-Host "  Updated: $updatedCount files" -ForegroundColor Green
Write-Host "  Errors: $errorCount files" -ForegroundColor $(if ($errorCount -gt 0) { "Red" } else { "Green" })
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "  1. Review the changes in each file"
Write-Host "  2. Build the project to check for compilation errors"
Write-Host "  3. Test dark mode with all control types"
Write-Host ""
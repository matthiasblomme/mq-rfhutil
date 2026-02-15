# PowerShell script to add OnEraseBkgnd handlers for gradient backgrounds
# This adds the handler to all 15 property page dialogs

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

Write-Host "Adding OnEraseBkgnd handlers for gradient backgrounds..." -ForegroundColor Cyan
Write-Host ""

$updatedHeaders = 0
$updatedCpp = 0
$errors = 0

foreach ($dialog in $dialogFiles) {
    $headerFile = "RFHUtil\$dialog.h"
    $cppFile = "RFHUtil\$dialog.cpp"
    
    Write-Host "  [PROCESSING] $dialog..." -ForegroundColor White
    
    # Update header file
    if (Test-Path $headerFile) {
        try {
            $headerContent = Get-Content $headerFile -Raw
            
            # Check if OnEraseBkgnd already exists
            if ($headerContent -notmatch "afx_msg\s+BOOL\s+OnEraseBkgnd") {
                # Add OnEraseBkgnd declaration after OnCtlColor
                if ($headerContent -match "(afx_msg\s+HBRUSH\s+OnCtlColor[^;]+;)") {
                    $headerContent = $headerContent -replace "(afx_msg\s+HBRUSH\s+OnCtlColor[^;]+;)", "`$1`r`n`tafx_msg BOOL OnEraseBkgnd(CDC* pDC);"
                    Set-Content -Path $headerFile -Value $headerContent -NoNewline
                    Write-Host "    [SUCCESS] Added OnEraseBkgnd declaration to header" -ForegroundColor Green
                    $updatedHeaders++
                } else {
                    Write-Host "    [SKIP] Could not find OnCtlColor in header" -ForegroundColor Yellow
                }
            } else {
                Write-Host "    [SKIP] OnEraseBkgnd already in header" -ForegroundColor Yellow
            }
        } catch {
            Write-Host "    [ERROR] Header: $_" -ForegroundColor Red
            $errors++
        }
    }
    
    # Update CPP file
    if (Test-Path $cppFile) {
        try {
            $cppContent = Get-Content $cppFile -Raw
            
            # Add ON_WM_ERASEBKGND() to message map if not present
            if ($cppContent -notmatch "ON_WM_ERASEBKGND\(\)") {
                if ($cppContent -match "(ON_WM_CTLCOLOR\(\))") {
                    $cppContent = $cppContent -replace "(ON_WM_CTLCOLOR\(\))", "`$1`r`n`tON_WM_ERASEBKGND()"
                    Write-Host "    [SUCCESS] Added ON_WM_ERASEBKGND() to message map" -ForegroundColor Green
                }
            }
            
            # Add OnEraseBkgnd implementation if not present
            if ($cppContent -notmatch "BOOL\s+$dialog\s*::\s*OnEraseBkgnd") {
                # Find OnCtlColor implementation and add OnEraseBkgnd after it
                if ($cppContent -match "(HBRUSH\s+$dialog\s*::\s*OnCtlColor[^}]+\})") {
                    $onCtlColorEnd = $matches[1]
                    $onEraseBkgndImpl = @"

BOOL $dialog::OnEraseBkgnd(CDC* pDC)
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
"@
                    $cppContent = $cppContent -replace [regex]::Escape($onCtlColorEnd), ($onCtlColorEnd + $onEraseBkgndImpl)
                    Set-Content -Path $cppFile -Value $cppContent -NoNewline
                    Write-Host "    [SUCCESS] Added OnEraseBkgnd implementation to CPP" -ForegroundColor Green
                    $updatedCpp++
                } else {
                    Write-Host "    [SKIP] Could not find OnCtlColor in CPP" -ForegroundColor Yellow
                }
            } else {
                Write-Host "    [SKIP] OnEraseBkgnd already in CPP" -ForegroundColor Yellow
            }
        } catch {
            Write-Host "    [ERROR] CPP: $_" -ForegroundColor Red
            $errors++
        }
    }
}

Write-Host ""
Write-Host "Summary:" -ForegroundColor Cyan
Write-Host "  Headers updated: $updatedHeaders" -ForegroundColor Green
Write-Host "  CPP files updated: $updatedCpp" -ForegroundColor Green
Write-Host "  Errors: $errors" -ForegroundColor $(if ($errors -gt 0) { "Red" } else { "Green" })
Write-Host ""
Write-Host "Next: Rebuild the project to test gradient backgrounds!" -ForegroundColor Yellow
Write-Host ""
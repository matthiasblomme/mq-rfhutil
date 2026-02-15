# PowerShell script to add OnCtlColor handler to dialogs that are missing it
# This adds the handler declaration, message map entry, and implementation

$dialogsToFix = @(
    "Dlq",
    "Ims", 
    "Props",
    "PS"
)

$onCtlColorImplementation = @'

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

Write-Host "Adding OnCtlColor handlers to dialogs..." -ForegroundColor Cyan
Write-Host ""

foreach ($dialog in $dialogsToFix) {
    $headerFile = "RFHUtil\$dialog.h"
    $cppFile = "RFHUtil\$dialog.cpp"
    
    Write-Host "  [PROCESSING] $dialog..." -ForegroundColor White
    
    # Check if files exist
    if (-not (Test-Path $headerFile)) {
        Write-Host "    [SKIP] Header file not found: $headerFile" -ForegroundColor Yellow
        continue
    }
    if (-not (Test-Path $cppFile)) {
        Write-Host "    [SKIP] CPP file not found: $cppFile" -ForegroundColor Yellow
        continue
    }
    
    try {
        # Read header file
        $headerContent = Get-Content $headerFile -Raw
        
        # Check if OnCtlColor declaration already exists
        if ($headerContent -match "afx_msg\s+HBRUSH\s+OnCtlColor") {
            Write-Host "    [SKIP] OnCtlColor already declared in header" -ForegroundColor Yellow
            continue
        }
        
        # Add OnCtlColor declaration to header (after other afx_msg declarations)
        if ($headerContent -match "(?s)(//\{\{AFX_MSG\([^)]+\).*?)(//\}\}AFX_MSG)") {
            $beforeEnd = $matches[1]
            $endMarker = $matches[2]
            $newDeclaration = "`tafx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);`r`n`t"
            $headerContent = $headerContent -replace [regex]::Escape($matches[0]), ($beforeEnd + $newDeclaration + $endMarker)
            Set-Content -Path $headerFile -Value $headerContent -NoNewline
            Write-Host "    [SUCCESS] Added OnCtlColor declaration to header" -ForegroundColor Green
        } else {
            Write-Host "    [ERROR] Could not find AFX_MSG section in header" -ForegroundColor Red
            continue
        }
        
        # Read CPP file
        $cppContent = Get-Content $cppFile -Raw
        
        # Add ON_WM_CTLCOLOR() to message map if not present
        if ($cppContent -notmatch "ON_WM_CTLCOLOR\(\)") {
            if ($cppContent -match "(?s)(BEGIN_MESSAGE_MAP.*?//\{\{AFX_MSG_MAP\([^)]+\))") {
                $messageMapStart = $matches[1]
                $replacement = $messageMapStart + "`r`n`tON_WM_CTLCOLOR()"
                $cppContent = $cppContent -replace [regex]::Escape($messageMapStart), $replacement
                Write-Host "    [SUCCESS] Added ON_WM_CTLCOLOR() to message map" -ForegroundColor Green
            } else {
                Write-Host "    [ERROR] Could not find message map in CPP" -ForegroundColor Red
                continue
            }
        }
        
        # Add OnCtlColor implementation after OnInitDialog
        if ($cppContent -match "BOOL\s+$dialog\s*::\s*OnInitDialog\s*\([^)]*\)\s*\{[^}]*\}") {
            $onInitDialog = $matches[0]
            $implementation = $onCtlColorImplementation -replace "CLASS_NAME", $dialog
            $cppContent = $cppContent -replace [regex]::Escape($onInitDialog), ($onInitDialog + $implementation)
            Set-Content -Path $cppFile -Value $cppContent -NoNewline
            Write-Host "    [SUCCESS] Added OnCtlColor implementation to CPP" -ForegroundColor Green
        } else {
            Write-Host "    [ERROR] Could not find OnInitDialog in CPP" -ForegroundColor Red
        }
        
    } catch {
        Write-Host "    [ERROR] $_" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "Done! Please review the changes and build the project." -ForegroundColor Cyan
Write-Host ""
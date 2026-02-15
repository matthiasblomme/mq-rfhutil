# PowerShell script to apply dark mode support to all remaining property page dialogs
# This script adds theme support to 13 remaining property pages

$dialogs = @(
    @{Name="MSGDATA"; Header="MSGDATA.h"; Source="MSGDATA.cpp"},
    @{Name="RFH"; Header="RFH.h"; Source="RFH.cpp"},
    @{Name="jms"; Header="jms.h"; Source="jms.cpp"},
    @{Name="CCICS"; Header="CICS.h"; Source="CICS.cpp"},
    @{Name="CDlq"; Header="Dlq.h"; Source="Dlq.cpp"},
    @{Name="CIms"; Header="Ims.h"; Source="Ims.cpp"},
    @{Name="other"; Header="other.h"; Source="other.cpp"},
    @{Name="CProps"; Header="Props.h"; Source="Props.cpp"},
    @{Name="CPS"; Header="PS.h"; Source="PS.cpp"},
    @{Name="pscr"; Header="pscr.h"; Source="pscr.cpp"},
    @{Name="PubSub"; Header="PubSub.h"; Source="PubSub.cpp"},
    @{Name="Usr"; Header="Usr.h"; Source="Usr.cpp"},
    @{Name="ConnSettings"; Header="ConnSettings.h"; Source="ConnSettings.cpp"}
)

$baseDir = "RFHUtil"

foreach ($dialog in $dialogs) {
    $className = $dialog.Name
    $headerFile = Join-Path $baseDir $dialog.Header
    $sourceFile = Join-Path $baseDir $dialog.Source
    
    Write-Host "Processing $className..." -ForegroundColor Cyan
    
    # 1. Add OnCtlColor declaration to header file
    if (Test-Path $headerFile) {
        $headerContent = Get-Content $headerFile -Raw
        
        # Find the message map section and add OnCtlColor
        if ($headerContent -match "(?s)(//\{\{AFX_MSG\($className\)\s*\n)(\s*afx_msg)") {
            $replacement = "`$1`tafx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);`n`$2"
            $headerContent = $headerContent -replace "(?s)(//\{\{AFX_MSG\($className\)\s*\n)(\s*afx_msg)", $replacement
            Set-Content $headerFile $headerContent -NoNewline
            Write-Host "  Updated $($dialog.Header)" -ForegroundColor Green
        }
    }
    
    # 2. Update source file
    if (Test-Path $sourceFile) {
        $sourceContent = Get-Content $sourceFile -Raw
        
        # Add ThemeManager include after other includes
        if ($sourceContent -notmatch '#include "ThemeManager.h"') {
            $sourceContent = $sourceContent -replace '(#include "stdafx.h"[^\n]*\n#include[^\n]*\n)', "`$1#include `"ThemeManager.h`"`n"
            Write-Host "  Added ThemeManager include" -ForegroundColor Green
        }
        
        # Add ON_WM_CTLCOLOR() to message map
        if ($sourceContent -match "(?s)(BEGIN_MESSAGE_MAP\($className[^\)]*\)\s*\n\s*//\{\{AFX_MSG_MAP\($className\)\s*\n)(\s*ON_)") {
            $replacement = "`$1`tON_WM_CTLCOLOR()`n`$2"
            $sourceContent = $sourceContent -replace "(?s)(BEGIN_MESSAGE_MAP\($className[^\)]*\)\s*\n\s*//\{\{AFX_MSG_MAP\($className\)\s*\n)(\s*ON_)", $replacement
            Write-Host "  Added ON_WM_CTLCOLOR() to message map" -ForegroundColor Green
        }
        
        # Find OnInitDialog and add theme application before return
        if ($sourceContent -match "(?s)(BOOL $className::OnInitDialog\(\)[^{]*\{.*?)(return TRUE;[^\}]*\})") {
            $beforeReturn = $matches[1]
            $returnStatement = $matches[2]
            
            if ($beforeReturn -notmatch "ApplyThemeToDialog") {
                $newContent = $beforeReturn + "`n`t// Apply theme to dialog`n`tThemeManager::GetInstance().ApplyThemeToDialog(this);`n`n`t" + $returnStatement
                $sourceContent = $sourceContent -replace "(?s)(BOOL $className::OnInitDialog\(\)[^{]*\{.*?)(return TRUE;[^\}]*\})", $newContent
                Write-Host "  Added ApplyThemeToDialog() call" -ForegroundColor Green
            }
        }
        
        # Add OnCtlColor implementation at the end of file (before last closing brace or at end)
        if ($sourceContent -notmatch "HBRUSH $className::OnCtlColor") {
            $onCtlColorImpl = @"

HBRUSH $className::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
	
	ThemeManager& theme = ThemeManager::GetInstance();
	
	if (theme.IsDarkMode())
	{
		pDC->SetTextColor(theme.GetTextColor());
		pDC->SetBkColor(theme.GetBackgroundColor());
		return theme.GetDialogBackgroundBrush();
	}
	
	return hbr;
}
"@
            $sourceContent = $sourceContent.TrimEnd() + "`n" + $onCtlColorImpl + "`n"
            Write-Host "  Added OnCtlColor() implementation" -ForegroundColor Green
        }
        
        Set-Content $sourceFile $sourceContent -NoNewline
        Write-Host "  Updated $($dialog.Source)" -ForegroundColor Green
    }
    
    Write-Host ""
}

Write-Host "Dark mode support applied to all dialogs!" -ForegroundColor Yellow
Write-Host "Please build the project to verify the changes." -ForegroundColor Yellow
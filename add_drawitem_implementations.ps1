# Add OnDrawItem implementations to all property pages

$propertyPages = @(
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

$implementation = @'

void CLASS_NAME::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// Only handle buttons
	if (lpDrawItemStruct->CtlType != ODT_BUTTON) {
		CPropertyPage::OnDrawItem(nIDCtl, lpDrawItemStruct);
		return;
	}
	
	ThemeManager& theme = ThemeManager::GetInstance();
	
	// Get button text
	CWnd* pWnd = GetDlgItem(nIDCtl);
	if (!pWnd) {
		CPropertyPage::OnDrawItem(nIDCtl, lpDrawItemStruct);
		return;
	}
	
	CString buttonText;
	pWnd->GetWindowText(buttonText);
	
	// Use ThemeManager to draw the button
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rect = lpDrawItemStruct->rcItem;
	
	theme.DrawThemedButton(pDC, rect, buttonText, lpDrawItemStruct->itemState);
}
'@

foreach ($page in $propertyPages) {
    $cppFile = "RFHUtil/$page.cpp"
    
    Write-Host "Processing $page..."
    
    if (Test-Path $cppFile) {
        $cppContent = Get-Content $cppFile -Raw
        
        # Check if OnDrawItem implementation already exists
        if ($cppContent -match "void ${page}::OnDrawItem") {
            Write-Host "  OnDrawItem implementation already exists in $cppFile"
            continue
        }
        
        # Find the OnEraseBkgnd implementation and add OnDrawItem after it
        $pattern = "(BOOL ${page}::OnEraseBkgnd\(CDC\* pDC\)\s*\{[^}]+\}[^}]+\})"
        
        if ($cppContent -match $pattern) {
            $implToAdd = $implementation -replace "CLASS_NAME", $page
            $replacement = '$1' + "`r`n" + $implToAdd
            $cppContent = $cppContent -replace $pattern, $replacement
            Set-Content -Path $cppFile -Value $cppContent -NoNewline
            Write-Host "  Added OnDrawItem implementation to $cppFile"
        } else {
            Write-Host "  [WARNING] Could not find OnEraseBkgnd implementation in $cppFile"
        }
    }
}

Write-Host "`nPhase 2 complete: Added OnDrawItem implementations"
Write-Host "Note: You still need to set BS_OWNERDRAW style on buttons in resource files or OnInitDialog"
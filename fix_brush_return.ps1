# Fix CBrush* to HBRUSH conversion in all OnCtlColor implementations

$files = @(
    "RFHUtil\General.cpp",
    "RFHUtil\MQMDPAGE.cpp",
    "RFHUtil\MSGDATA.cpp",
    "RFHUtil\RFH.cpp",
    "RFHUtil\jms.cpp",
    "RFHUtil\CICS.cpp",
    "RFHUtil\Dlq.cpp",
    "RFHUtil\Ims.cpp",
    "RFHUtil\other.cpp",
    "RFHUtil\Props.cpp",
    "RFHUtil\PS.cpp",
    "RFHUtil\pscr.cpp",
    "RFHUtil\PubSub.cpp",
    "RFHUtil\Usr.cpp",
    "RFHUtil\ConnSettings.cpp"
)

foreach ($file in $files) {
    if (Test-Path $file) {
        $content = Get-Content $file -Raw
        
        # Replace the return statement to cast CBrush* to HBRUSH
        $content = $content -replace 'return theme\.GetDialogBackgroundBrush\(\);', 'return (HBRUSH)theme.GetDialogBackgroundBrush()->GetSafeHandle();'
        
        Set-Content $file $content -NoNewline
        Write-Host "Fixed $file" -ForegroundColor Green
    }
}

Write-Host "`nAll files fixed!" -ForegroundColor Yellow
# Setup MSBuild System-Wide Alias
# This script adds MSBuild to the system PATH for easy access

Write-Host "Setting up MSBuild system-wide access..." -ForegroundColor Cyan

$msbuildPath = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin"

# Check if MSBuild exists at this path
if (Test-Path "$msbuildPath\MSBuild.exe") {
    Write-Host "[OK] MSBuild found at: $msbuildPath" -ForegroundColor Green
    
    # Get current system PATH
    $currentPath = [Environment]::GetEnvironmentVariable("Path", "Machine")
    
    # Check if already in PATH
    if ($currentPath -like "*$msbuildPath*") {
        Write-Host "[OK] MSBuild is already in system PATH" -ForegroundColor Green
    } else {
        Write-Host "Adding MSBuild to system PATH..." -ForegroundColor Yellow
        
        # Add to system PATH (requires admin)
        try {
            $newPath = $currentPath + ";" + $msbuildPath
            [Environment]::SetEnvironmentVariable("Path", $newPath, "Machine")
            Write-Host "[OK] MSBuild added to system PATH successfully" -ForegroundColor Green
            Write-Host "  Note: You need to restart your terminal for changes to take effect" -ForegroundColor Yellow
        } catch {
            Write-Host "[ERROR] Failed to update system PATH. Please run as Administrator." -ForegroundColor Red
            Write-Host "  Error: $_" -ForegroundColor Red
        }
    }
    
    # Also add to current session
    $env:Path += ";$msbuildPath"
    Write-Host "[OK] MSBuild added to current session PATH" -ForegroundColor Green
    
    # Test MSBuild
    Write-Host "`nTesting MSBuild..." -ForegroundColor Cyan
    & msbuild -version
    
    Write-Host "`n[OK] Setup complete!" -ForegroundColor Green
    Write-Host "You can now use 'msbuild' command directly" -ForegroundColor Green
    Write-Host "Note: New terminals will need to be restarted to pick up the PATH change" -ForegroundColor Yellow
    
} else {
    Write-Host "[ERROR] MSBuild not found at: $msbuildPath" -ForegroundColor Red
    Write-Host "Please verify your Visual Studio installation" -ForegroundColor Red
}
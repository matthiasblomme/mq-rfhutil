$base = 'd:\GIT\mq-rfhutil'
New-Item -ItemType Directory -Force -Path "$base\bin\tests\x64" | Out-Null

function Run-Tests {
    param($exe, $xml, $label)
    Write-Host ""
    Write-Host "=== $label ===" -ForegroundColor Cyan
    if (-not (Test-Path $exe)) {
        Write-Host "MISSING: $exe" -ForegroundColor Red
        return $false
    }
    $p = Start-Process -FilePath $exe -ArgumentList "--gtest_output=xml:$xml" -Wait -PassThru -NoNewWindow
    $code = $p.ExitCode
    if (Test-Path $xml) {
        $x = [xml](Get-Content $xml)
        $ts = $x.testsuites
        Write-Host "Tests: $($ts.tests)  Failures: $($ts.failures)  Errors: $($ts.errors)"
        foreach ($suite in $x.testsuites.testsuite) {
            foreach ($tc in $suite.testcase) {
                if ($tc.failure) {
                    Write-Host "FAILED: $($suite.name).$($tc.name)" -ForegroundColor Red
                    Write-Host $tc.failure.'#text' -ForegroundColor Yellow
                }
            }
        }
    } else {
        Write-Host "No XML produced" -ForegroundColor Red
    }
    Write-Host "Exit: $code"
    return ($code -eq 0)
}

$r32 = Run-Tests "$base\bin\tests\UnitTests.exe"     "$base\bin\tests\results_win32.xml"   "Win32 Debug"
$r64 = Run-Tests "$base\bin\tests\x64\UnitTests.exe" "$base\bin\tests\x64\results_x64.xml" "x64 Debug"
Write-Host ""
if ($r32 -and $r64) { Write-Host "ALL TESTS PASSED" -ForegroundColor Green; exit 0 }
else                 { Write-Host "SOME TESTS FAILED" -ForegroundColor Red;  exit 1 }

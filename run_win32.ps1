$p = Start-Process -FilePath 'd:\GIT\mq-rfhutil\bin\tests\UnitTests.exe' `
     -ArgumentList '--gtest_color=no' `
     -Wait -PassThru `
     -RedirectStandardOutput 'd:\GIT\mq-rfhutil\bin\tests\out_win32.txt' `
     -RedirectStandardError  'd:\GIT\mq-rfhutil\bin\tests\err_win32.txt'
Write-Host "Exit: $($p.ExitCode)"
Get-Content 'd:\GIT\mq-rfhutil\bin\tests\out_win32.txt'
Get-Content 'd:\GIT\mq-rfhutil\bin\tests\err_win32.txt'

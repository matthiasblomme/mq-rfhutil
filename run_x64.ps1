$p = Start-Process -FilePath 'd:\GIT\mq-rfhutil\bin\tests\x64\UnitTests.exe' `
     -ArgumentList '--gtest_color=no' `
     -Wait -PassThru `
     -RedirectStandardOutput 'd:\GIT\mq-rfhutil\bin\tests\x64\out_x64.txt' `
     -RedirectStandardError  'd:\GIT\mq-rfhutil\bin\tests\x64\err_x64.txt'
Write-Host "Exit: $($p.ExitCode)"
Get-Content 'd:\GIT\mq-rfhutil\bin\tests\x64\out_x64.txt'
Get-Content 'd:\GIT\mq-rfhutil\bin\tests\x64\err_x64.txt'

# Build Configuration for mq-rfhutil

## MSBuild Path
**IMPORTANT:** Use this MSBuild path for building the project:
```
C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe
```

## Build Commands

Always build both projects!

### Build RFHUtil (main project)
```bash
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" RFHUtil.sln /t:RFHUtil:Rebuild /p:Configuration=Release /p:Platform=Win32 /v:minimal
```

### Build Client (rfhutilc)
```bash
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" RFHUtil.sln /t:Client:Rebuild /p:Configuration=Release /p:Platform=Win32 /v:minimal
```

### Build Both Projects
```bash
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" RFHUtil.sln /t:Rebuild /p:Configuration=Release /p:Platform=Win32 /v:minimal
```

## Output Locations
- **rfhutil.exe**: `bin\Release\rfhutil.exe`
- **rfhutilc.exe**: `bin\Release\rfhutilc.exe`

## Project Structure
- **RFHUtil**: Main GUI application (server mode)
- **Client**: Client mode application (rfhutilc.exe)
- Both projects share source files from `RFHUtil\` directory

## Current Version
**9.4.0.0** - Built against IBM MQ 9.4.5
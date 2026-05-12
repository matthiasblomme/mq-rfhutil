# Build Configuration for mq-rfhutil

## Prerequisites

### IBM MQ SDK
The build requires the IBM MQ SDK (headers + libraries). Set the `MQ_HOME` environment variable to your MQ installation root before building:

```cmd
set MQ_HOME=C:\Program Files\IBM\MQ
```

Or pass it directly to MSBuild with `/p:MQ_HOME=<path>` (see commands below).

The default in `Directory.Build.props` is `C:\Program Files\IBM\MQ` (standard IBM MQ install location). If your MQ SDK is elsewhere (e.g. `d:\apps\mq`), you must override it.

### MSBuild Path
**IMPORTANT:** Use this MSBuild path for building the project:
```
C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe
```

## Build Commands

### Build RFHUtil (main project - server mode)
```bash
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" RFHUtil.sln /t:RFHUtil:Rebuild /p:Configuration=Release /p:Platform=Win32 /v:minimal
```

### Build Client (rfhutilc - client mode)
```bash
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" RFHUtil.sln /t:Client:Rebuild /p:Configuration=Release /p:Platform=Win32 /v:minimal
```

### Build Client Safe Mode (rfhutilc-safe - browse-only)
```bash
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" RFHUtil.sln /t:Client:Rebuild /p:Configuration=ReleaseSafe /p:Platform=Win32 /v:minimal
```

### Build All Projects
```bash
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" RFHUtil.sln /t:Rebuild /p:Configuration=Release /p:Platform=Win32 /v:minimal
```

### Override MQ SDK path (non-standard install location)
```bash
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" RFHUtil.sln /t:Rebuild /p:Configuration=Release /p:Platform=Win32 /p:MQ_HOME="d:\apps\mq" /v:minimal
```

## Build Configurations

### Release (Standard)
- **rfhutil.exe**: Full GUI application with MQ server libraries
- **rfhutilc.exe**: Client mode with MQ client libraries
- Full read/write capabilities

### ReleaseSafe (Browse-Only)
- **rfhutilc-safe.exe**: Client mode with SAFE_MODE enabled
- Browse-only operations (non-destructive reads)
- All write operations disabled (Write Q, Load Q, Move Q, Purge Q, Clear All)
- Ideal for production troubleshooting and training environments

## Output Locations
- **rfhutil.exe**: `bin\Release\rfhutil.exe`
- **rfhutilc.exe**: `bin\Release\rfhutilc.exe`
- **rfhutilc-safe.exe**: `bin\ReleaseSafe\rfhutilc-safe.exe`

## Project Structure
- **RFHUtil**: Main GUI application (server mode)
- **Client**: Client mode application (rfhutilc.exe / rfhutilc-safe.exe)
- Both projects share source files from `RFHUtil\` directory

## Safe Mode Features
When built with `ReleaseSafe` configuration:
- **SAFE_MODE** preprocessor definition is active
- Write Q button is hidden and disabled
- Load Q button is hidden and disabled
- Move Q button is hidden and disabled
- Purge Q button is hidden and disabled
- Clear All button is hidden and disabled
- Read Q button renamed to "Browse Q" and uses non-destructive browse mode
- All MQGET operations automatically use browse mode (MQGMO_BROWSE_*)
- Save Q (to file) remains enabled
- Browse operations remain fully functional

See [SAFE_MODE_IMPLEMENTATION.md](SAFE_MODE_IMPLEMENTATION.md) for detailed implementation information.

## Cutting a Release

The release workflow (`.github/workflows/release.yml`) triggers on any tag
matching `v*` and publishes the committed Release/ReleaseSafe binaries.

1. Rebuild and commit Release + ReleaseSafe binaries locally (see build
   commands above).
2. Tag and push:
   ```bash
   git tag v9.4.0.0
   git push origin v9.4.0.0
   ```
3. The workflow attaches `rfhutil.exe`, `rfhutilc.exe`, and
   `rfhutilc-safe.exe` to an auto-generated GitHub Release.

The runner has no IBM MQ SDK, so the workflow does not build — it publishes
what is tracked at the tagged commit. Always rebuild and commit binaries
before tagging.

## Current Version
**9.4.0.0** - Built against IBM MQ 9.4.5
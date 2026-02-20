# mq-rfhutil

This is a fork of the **rfhutil** program, originally released in SupportPac IH03 and maintained by IBM Messaging.

**Original Repository**: [ibm-messaging/mq-rfhutil](https://github.com/ibm-messaging/mq-rfhutil)

This program can be useful for the development and testing of IBM MQ and
IBM Integration Bus aka WebSphere Message Broker aka App Connect Enterprise applications.
Test messages are stored as files, which are then read by the application and written to an MQ queue. The program is GUI based.

## Latest Features (v9.4.0.0)

### 🔒 Safe Mode (Browse-Only Version) - NEW!

This fork adds a **Safe Mode** build configuration that creates a browse-only version (`rfhutilc-safe.exe`) with all write operations disabled.

#### Key Features
- **Browse-Only Operations**: All message reads use non-destructive browse mode
- **Disabled Write Operations**: Write Q, Load Q, Move Q, Purge Q, and Clear All are completely disabled
- **Production-Safe**: Perfect for troubleshooting production environments without risk of data modification
- **Training-Friendly**: Ideal for training new team members without fear of accidental changes
- **Compliance-Ready**: Meets requirements for read-only access in regulated environments

#### What's Disabled in Safe Mode
- ❌ Write Q - Cannot write messages to queues
- ❌ Load Q - Cannot load messages from files to queues
- ❌ Move Q - Cannot move messages between queues
- ❌ Purge Q - Cannot clear messages from queues
- ❌ Clear All - Cannot clear all data
- ❌ Destructive Reads - All reads automatically use browse mode

#### What's Still Available
- ✅ Browse Operations - Start Browse, Browse Next, Browse Previous
- ✅ Save Q - Save messages to files (read-only operation)
- ✅ Display Q - View queue information
- ✅ Read Files - Load and view message data from files
- ✅ All Analysis Tools - Parse, format, and analyze message content

#### Building Safe Mode
```bash
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" RFHUtil.sln /t:Client:Rebuild /p:Configuration=ReleaseSafe /p:Platform=Win32 /v:minimal
```

Output: `bin\ReleaseSafe\rfhutilc-safe.exe`

See [SAFE_MODE_IMPLEMENTATION.md](docs/SAFE_MODE_IMPLEMENTATION.md) and [BUILD_CONFIG.md](docs/BUILD_CONFIG.md) for detailed implementation information.

## Original Features (v9.4.0.0)

### 🎨 Dark Mode Support
RFHUtil now includes full dark mode support with automatic theme detection and manual theme switching.

**Light Mode:**
![Light Mode](docs/screenshots/light-mode.png)

**Dark Mode:**
![Dark Mode](docs/screenshots/dark-mode.png)

**Theme Menu:**
![Theme Selection](docs/screenshots/theme-menu.png)

### 🔄 Enhanced Connection Reliability

RFHUtil now features intelligent automatic reconnection that seamlessly handles queue manager restarts and connection losses.

#### Key Features
- **HeartBeat/KeepAlive Configuration**: Fine-tune connection monitoring for optimal reliability
- **Automatic Reconnection**: Seamlessly reconnects to queue managers with configurable retry logic
- **Single-Click Recovery**: Operations complete in one click after reconnection - no manual intervention needed
- **Browse Operation Recovery**: Automatically restarts browse operations after reconnection
- **Connection Settings UI**: New dedicated tab for managing all connection parameters

#### How It Works

When a connection is lost (e.g., queue manager restart), RFHUtil automatically:
1. Detects the connection failure
2. Attempts to reconnect to the queue manager
3. Reopens the queue
4. Completes the requested operation
5. All in a **single button click** - no error dialogs, no manual reconnection needed!

**Read Q Operation Flow:**
```mermaid
sequenceDiagram
    participant User
    participant RFHUtil
    participant QM as Queue Manager
    
    User->>RFHUtil: Click "Read Q"
    RFHUtil->>QM: MQGET request
    Note over QM: Connection Lost!
    QM-->>RFHUtil: MQRC_CONNECTION_BROKEN (2009)
    RFHUtil->>RFHUtil: Detect connection loss
    RFHUtil->>QM: Reconnect to QM
    QM-->>RFHUtil: Connected
    RFHUtil->>QM: Reopen queue
    RFHUtil->>QM: Retry MQGET
    QM-->>RFHUtil: Message data
    RFHUtil->>User: Display message
    Note over User,RFHUtil: All in single click!
```

**Browse Operation Flow:**
```mermaid
sequenceDiagram
    participant User
    participant RFHUtil
    participant QM as Queue Manager
    
    User->>RFHUtil: Click "Start Browse"
    RFHUtil->>QM: Open queue for browse
    QM-->>RFHUtil: First message
    Note over QM: QM Restarted
    User->>RFHUtil: Click "Browse Next"
    RFHUtil->>QM: MQGET BROWSE_NEXT
    QM-->>RFHUtil: MQRC_CONNECTION_BROKEN (2009)
    RFHUtil->>RFHUtil: Detect connection loss
    RFHUtil->>QM: Reconnect to QM
    QM-->>RFHUtil: Connected
    RFHUtil->>QM: Restart browse from first
    QM-->>RFHUtil: First message
    RFHUtil->>User: Display message
    Note over User,RFHUtil: Browse continues seamlessly!
```

#### Benefits
- ✅ **No manual reconnection** - Everything happens automatically
- ✅ **No error popups** - Status messages appear in the log window
- ✅ **Single click operation** - No need to click twice after reconnection
- ✅ **Browse state preserved** - Browse operations continue from where they left off
- ✅ **Configurable retry logic** - Customize reconnection attempts and intervals

**Connection Settings Tab:**
<!-- TODO: Add screenshot of connection settings tab -->
![Connection Settings](docs/screenshots/connection-settings.png)

**Reconnection in Action:**
<!-- TODO: Add screenshot showing reconnection log messages -->
![Auto Reconnection](docs/screenshots/auto-reconnection.png)

## Possible Uses
It allows test messages to be captured and stored in files, and then used to drive Message Flows. Output messages can also be read and displayed in a variety of formats. The formats include two types of XML as well as matched against a COBOL copybook. The data can be in EBCDIC or ASCII. An RFH2 header can be added to the message before the message is sent.

## Skill Level Required
None beyond basic MQ and IIB/ACE development skills.

## Contents of repository
This repository contains source code for the rfhutil program, managed as a Microsoft Visual Studio 2017 Solution.
If you have VS 2017, then opening the `RFHUtil.sln` file will allow you to rebuild the program.

Pre-built copies of the programs are available:
- **rfhutil.exe** - Full version for connections to a local queue manager (`bin\Release`)
- **rfhutilc.exe** - Client version for MQ client connections (`bin\Release`)
- **rfhutilc-safe.exe** - **NEW!** Browse-only safe mode version (`bin\ReleaseSafe`)

They can be run directly but you may first need to run the `setmqenv` program to set the environment variables that allow you to locate the MQ runtime libraries.

Documentation is provided in the `ih03.doc` and `ih03.pdf` files.

## Building and running the programs
The VS 2017 configuration assumes that MQ is installed in the default location, C:\Program Files\IBM\MQ. If you have installed MQ
elsewhere, then you may consider adding a link from the default location to your installation directory. Otherwise, you will
have to modify the configuration properties.

Running the programs may require that you run `setmqenv` to set a suitable environment for the programs to locate the MQ
libraries.

### Performance testing tools
The IH03 SupportPac also included some independent performance testing programs. Those programs are included in this repository under
the `mqperf` subdirectory. There is a Visual Studio 2017 Solution configuration to rebuild those tools included. Generated executables
are also shipped in the `bin\Release` tree.

The performance testing programs are provided in a **single configuration**, linked with the mqm.dll. That can work for both client connections
and local queue managers. To **force** connections to be made across a client channel, then set the `MQ_CONNECT_TYPE` environment
variable to `CLIENT`.


## History
The **rfhutil** program was conceived, created and developed by **Jim MacNair**.

See [CHANGELOG](CHANGELOG.md) for changes to the program since its inception. The first release to Github is called version 9.1, and was released in December 2018.

### This Fork
This fork adds Safe Mode (browse-only) functionality to provide a production-safe version of rfhutilc for environments where read-only access is required.

**Original Repository**: [ibm-messaging/mq-rfhutil](https://github.com/ibm-messaging/mq-rfhutil)

## Health Warning

This package is provided as-is with no guarantees of support or updates.

## Issues and Contributions

For issues relating to the **original rfhutil**, please use the [IBM Messaging issue tracker](https://github.com/ibm-messaging/mq-rfhutil/issues).

For issues relating to the **Safe Mode feature** in this fork, please use this repository's issue tracker.

Contributions to the original package can be accepted under the terms of the
IBM Contributor License Agreement, found in the file [CLA.md](CLA.md) of this repository.

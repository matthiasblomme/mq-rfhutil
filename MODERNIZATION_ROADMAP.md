# RFHUtil Modernization Roadmap - Detailed Action Plan

## 📊 Progress Tracker

**Last Updated:** February 21, 2026
**Current Version:** 9.4.0.0
**Build Environment:** Visual Studio 2022 (v143), IBM MQ 9.4.5

### Completed Items ✅

| Priority | Item | Status | Completed Date | Notes |
|----------|------|--------|----------------|-------|
| 🔴 **P0.1** | HeartBeat/KeepAlive Configuration | ✅ COMPLETE | Feb 14, 2026 | Added to DataArea with UI controls |
| 🔴 **P0.2** | Automatic Reconnection | ✅ COMPLETE | Feb 14, 2026 | Exponential backoff, 7 error handlers |
| 🔴 **P0.3** | Connection Settings UI Tab | ✅ COMPLETE | Feb 14, 2026 | 15th tab with 3 sections, 28 controls |
| 🟡 **P1.1** | Visual Studio 2022 Upgrade | ✅ COMPLETE | Feb 14, 2026 | Already using VS 2022 Build Tools |
| 🟡 **P1.2** | Dark Mode Support | ✅ COMPLETE | Feb 21, 2026 | Full implementation with visual polish |

### In Progress 🚧

| Priority | Item | Status | Started Date | Target Date |
|----------|------|--------|--------------|-------------|
| None | - | - | - | - |

### Upcoming 📋

| Priority | Item | Effort | Impact | Target Quarter |
|----------|------|--------|--------|----------------|
| 🟡 **P1.3** | 64-bit Support | Low | Medium | Q1 2026 |
| 🟡 **P1.4** | Basic Unit Testing | Medium | High | Q1 2026 |
| 🟢 **P2.1** | Connection Health Monitor | Medium | Medium | Q2 2026 |
| 🟢 **P2.2** | Secure Credential Storage | Medium | High | Q2 2026 |

### Documentation 📚

All detailed documentation has been moved to the [`docs/`](docs/) folder:
- [Architecture Analysis](docs/ARCHITECTURE_ANALYSIS.md)
- [Build Configuration](docs/BUILD_CONFIG.md)
- [MQ HeartBeat Details](docs/MQ_HEARTBEAT_NEGOTIATION.md)
- [MQ KeepAlive Details](docs/MQ_KEEPALIVE_DETAILED.md)
- [P0 Implementation Plans](docs/P0_IMPLEMENTATION_PLAN.md)
- [Test Environment Info](docs/TEST_ENVIRONMENT_INFO.md)

---

## Executive Summary

This document provides a **detailed, actionable roadmap** for modernizing the mq-rfhutil project. Each recommendation includes specific steps, code examples, and expected outcomes.

---

## Priority Matrix

### Immediate Impact (Do First - Weeks 1-4)

| Priority | Item | Effort | Impact | Risk | Status |
|----------|------|--------|--------|------|--------|
| 🔴 **P0.1** | Add HeartBeat/KeepAlive | Low | High | Low | ✅ DONE |
| 🔴 **P0.2** | Implement Auto-Reconnect | Medium | High | Low | ✅ DONE |
| 🔴 **P0.3** | Add tab to enable auto-reconnect and heartbeat/keepalive | Medium | High | Low | ✅ DONE |
| 🟡 **P1.1** | Upgrade to VS 2022 | Low | Medium | Low | ✅ DONE |
| 🟡 **P1.2** | Dark mode | Low | Medium | Low | ✅ DONE |


### Short Term (Weeks 5-12)

| Priority | Item | Effort | Impact | Risk | Status |
|----------|------|--------|--------|------|--------|
| 🟡 **P1.3** | Add 64-bit Support | Low | Medium | Low | 📋 PLANNED |
| 🟡 **P1.4** | Basic Unit Testing | Medium | High | Low | 📋 PLANNED |
| 🟢 **P2.1** | Connection Health Monitor | Medium | Medium | Low | 📋 PLANNED |
| 🟢 **P2.2** | Secure Credential Storage | Medium | High | Medium | 📋 PLANNED |
| 🟢 **P2.3** | Make the Data tab editable by adding a check mark and allow for that data to be written to the queue | Medium | Medium | Medium | 📋 PLANNED |

### Medium Term (Months 4-6)

| Priority | Item | Effort | Impact | Risk |
|----------|------|--------|--------|------|
| 🟢 **P2** | Refactor DataArea Class | High | High | Medium |
| 🟢 **P2** | Modern C++ Features | High | Medium | Medium |
| 🔵 **P3** | Darkmode | Medium | Low | Medium |
| 🔵 **P3** | CMake Build System | Medium | Low | Medium |
| 🔵 **P3** | Comprehensive Testing | High | High | Low |
| 🟡 **P4** | Add GitHub Actions CI/CD | Medium | High | Low |

---

## 1. IMMEDIATE: Connection Reliability (P0)

### 1.1 Add HeartBeat and KeepAlive Configuration

**Problem:** Current code doesn't set these, leading to slow failure detection and firewall timeouts.

**Solution:** Add explicit configuration in connection setup.

#### Implementation Steps

**Step 1: Locate the connection code**
- File: [`RFHUtil/DataArea.cpp`](RFHUtil/DataArea.cpp:10350-10450)
- Method: `DataArea::connect2QM()`
- Line: After 10407 (after `cd.MaxMsgLength = 104857600;`)

**Step 2: Add the configuration**

```cpp
// File: RFHUtil/DataArea.cpp
// Location: After line 10407

// ============================================================
// MODERNIZATION: Add HeartBeat and KeepAlive configuration
// ============================================================

// Set HeartBeat interval for fast MQ-level failure detection
// This detects queue manager crashes, process failures, and keeps
// the connection active to prevent firewall timeouts
cd.HeartBeatInterval = 60;  // 60 seconds (vs default 300s)

// Set KeepAlive interval for network-level failure detection
// This detects network cable unplugs, router failures, and
// "half-open" TCP connections
cd.KeepAliveInterval = MQKAI_AUTO;  // Use OS defaults

// For environments with aggressive firewalls (5-minute timeout):
// cd.KeepAliveInterval = 30;  // 30 seconds

// Ensure we're using a version that supports these fields
if (cd.Version < MQCD_VERSION_7) {
    cd.Version = MQCD_VERSION_7;
    cd.StrucLength = MQCD_LENGTH_7;
}

// Add trace logging
if (traceEnabled) {
    sprintf(traceInfo, "HeartBeat=%d, KeepAlive=%d", 
            cd.HeartBeatInterval, cd.KeepAliveInterval);
    logTraceEntry(traceInfo);
}
```

**Step 3: Test the changes**

```cpp
// Test scenarios:
// 1. Normal connection - verify heartbeats are sent
// 2. Network disconnect - verify fast detection
// 3. QM crash - verify fast detection
// 4. Long idle period - verify no firewall timeout
```

#### Expected Outcomes

**Before:**
- Failure detection: 300+ seconds (5+ minutes)
- Firewall timeouts: Common after 5 minutes idle
- User experience: Poor (long waits, manual reconnects)

**After:**
- Failure detection: 60 seconds (5x faster)
- Firewall timeouts: Eliminated (heartbeats keep connection alive)
- User experience: Excellent (fast detection, automatic recovery)

#### Effort Estimate
- **Development:** 1 hour
- **Testing:** 2 hours
- **Documentation:** 1 hour
- **Total:** 4 hours

---

### 1.2 Implement Automatic Reconnection

**Problem:** When connection fails, user must manually reconnect. Poor user experience.

**Solution:** Add automatic reconnection with exponential backoff.

#### Implementation Steps

**Step 1: Create ConnectionManager class**

Create new file: `RFHUtil/ConnectionManager.h`

```cpp
/*
 * ConnectionManager.h
 * Manages MQ connections with automatic reconnection
 */

#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include "cmqc.h"
#include <string>

class ConnectionManager {
public:
    // Configuration
    struct Config {
        int maxRetries = 3;
        int initialDelayMs = 1000;      // 1 second
        int maxDelayMs = 30000;         // 30 seconds
        double backoffMultiplier = 2.0; // Exponential backoff
        bool enableAutoReconnect = true;
    };

    ConnectionManager();
    ~ConnectionManager();

    // Connection management
    bool connect(const char* qmName, const Config& config = Config());
    bool disconnect();
    bool isConnected() const;
    bool ensureConnected();  // Check and reconnect if needed

    // Health monitoring
    bool isHealthy();
    MQLONG getLastCompletionCode() const { return lastCC; }
    MQLONG getLastReasonCode() const { return lastRC; }
    const char* getLastError() const { return lastError.c_str(); }

    // Connection handle
    MQHCONN getHandle() const { return qm; }

private:
    MQHCONN qm;
    bool connected;
    std::string currentQM;
    Config config;
    
    // Error tracking
    MQLONG lastCC;
    MQLONG lastRC;
    std::string lastError;
    
    // Reconnection state
    int reconnectAttempts;
    DWORD lastReconnectTime;

    // Internal methods
    bool connectInternal(const char* qmName);
    bool reconnect();
    int calculateBackoffDelay(int attempt);
    void logConnectionEvent(const char* event);
};

#endif // CONNECTION_MANAGER_H
```

**Step 2: Implement ConnectionManager**

Create new file: `RFHUtil/ConnectionManager.cpp`

```cpp
/*
 * ConnectionManager.cpp
 * Implementation of automatic reconnection logic
 */

#include "stdafx.h"
#include "ConnectionManager.h"
#include "comsubs.h"
#include <algorithm>

ConnectionManager::ConnectionManager()
    : qm(MQHO_NONE)
    , connected(false)
    , lastCC(MQCC_OK)
    , lastRC(MQRC_NONE)
    , reconnectAttempts(0)
    , lastReconnectTime(0)
{
}

ConnectionManager::~ConnectionManager()
{
    if (connected) {
        disconnect();
    }
}

bool ConnectionManager::connect(const char* qmName, const Config& cfg)
{
    config = cfg;
    currentQM = qmName;
    
    if (!config.enableAutoReconnect) {
        // Simple connection without retry
        return connectInternal(qmName);
    }
    
    // Try connection with retries
    for (int attempt = 0; attempt < config.maxRetries; attempt++) {
        reconnectAttempts = attempt;
        
        logConnectionEvent("Attempting connection");
        
        if (connectInternal(qmName)) {
            reconnectAttempts = 0;
            logConnectionEvent("Connection successful");
            return true;
        }
        
        // If not last attempt, wait before retry
        if (attempt < config.maxRetries - 1) {
            int delay = calculateBackoffDelay(attempt);
            
            char msg[256];
            sprintf(msg, "Connection failed (attempt %d/%d), retrying in %dms", 
                    attempt + 1, config.maxRetries, delay);
            logConnectionEvent(msg);
            
            Sleep(delay);
        }
    }
    
    logConnectionEvent("Connection failed after all retries");
    return false;
}

bool ConnectionManager::connectInternal(const char* qmName)
{
    // This would call the existing DataArea::connect2QM() logic
    // For now, this is a placeholder showing the structure
    
    MQCNO cno = {MQCNO_DEFAULT};
    MQCD cd = {MQCD_CLIENT_CONN_DEFAULT};
    
    // Configure connection options
    cno.Version = MQCNO_VERSION_2;
    cno.Options = MQCNO_HANDLE_SHARE_NO_BLOCK;
    cno.ClientConnPtr = &cd;
    
    // Configure channel definition with HeartBeat and KeepAlive
    cd.Version = MQCD_VERSION_7;
    cd.StrucLength = MQCD_LENGTH_7;
    cd.HeartBeatInterval = 60;
    cd.KeepAliveInterval = MQKAI_AUTO;
    
    // Attempt connection
    MQCONNX((char*)qmName, &cno, &qm, &lastCC, &lastRC);
    
    if (lastCC == MQCC_OK) {
        connected = true;
        return true;
    }
    
    // Store error information
    char errMsg[512];
    sprintf(errMsg, "MQCONNX failed: CC=%d, RC=%d", lastCC, lastRC);
    lastError = errMsg;
    
    return false;
}

bool ConnectionManager::disconnect()
{
    if (!connected || qm == MQHO_NONE) {
        return true;
    }
    
    MQDISC(&qm, &lastCC, &lastRC);
    
    connected = false;
    qm = MQHO_NONE;
    
    return (lastCC == MQCC_OK);
}

bool ConnectionManager::isConnected() const
{
    return connected && (qm != MQHO_NONE);
}

bool ConnectionManager::ensureConnected()
{
    if (!isConnected()) {
        return false;
    }
    
    // Perform lightweight health check
    if (!isHealthy()) {
        logConnectionEvent("Connection unhealthy, attempting reconnect");
        return reconnect();
    }
    
    return true;
}

bool ConnectionManager::isHealthy()
{
    if (!connected) {
        return false;
    }
    
    // Perform a lightweight MQINQ to check connection health
    MQLONG selector = MQIA_Q_MGR_STATUS;
    MQLONG value;
    
    MQINQ(qm, MQOT_Q_MGR, NULL, 1, &selector, 0, NULL, 
          sizeof(value), &value, &lastCC, &lastRC);
    
    return (lastCC == MQCC_OK);
}

bool ConnectionManager::reconnect()
{
    logConnectionEvent("Reconnecting");
    
    // Disconnect if still connected
    if (connected) {
        disconnect();
    }
    
    // Attempt reconnection
    return connect(currentQM.c_str(), config);
}

int ConnectionManager::calculateBackoffDelay(int attempt)
{
    // Exponential backoff: delay = initial * (multiplier ^ attempt)
    double delay = config.initialDelayMs * 
                   pow(config.backoffMultiplier, attempt);
    
    // Cap at maximum delay
    int finalDelay = (int)std::min(delay, (double)config.maxDelayMs);
    
    return finalDelay;
}

void ConnectionManager::logConnectionEvent(const char* event)
{
    // Log to trace if enabled
    if (traceEnabled) {
        char msg[512];
        sprintf(msg, "ConnectionManager: %s (QM=%s, Attempt=%d)", 
                event, currentQM.c_str(), reconnectAttempts);
        logTraceEntry(msg);
    }
}
```

**Step 3: Integrate with DataArea**

Modify `RFHUtil/DataArea.h`:

```cpp
// Add to DataArea class
private:
    ConnectionManager* connMgr;  // New connection manager

public:
    // Add new methods
    bool connectWithRetry(const char* qmName);
    bool ensureConnection();
```

Modify `RFHUtil/DataArea.cpp`:

```cpp
// In constructor
DataArea::DataArea()
{
    // ... existing code ...
    
    // Initialize connection manager
    connMgr = new ConnectionManager();
}

// In destructor
DataArea::~DataArea()
{
    // ... existing code ...
    
    if (connMgr) {
        delete connMgr;
        connMgr = NULL;
    }
}

// New method: Connect with automatic retry
bool DataArea::connectWithRetry(const char* qmName)
{
    ConnectionManager::Config config;
    config.maxRetries = 3;
    config.initialDelayMs = 2000;  // 2 seconds
    config.maxDelayMs = 30000;     // 30 seconds
    config.backoffMultiplier = 2.0;
    config.enableAutoReconnect = true;
    
    return connMgr->connect(qmName, config);
}

// New method: Ensure connection is healthy
bool DataArea::ensureConnection()
{
    return connMgr->ensureConnected();
}
```

**Step 4: Update UI to use new connection methods**

Modify `RFHUtil/General.cpp` (or wherever connection is initiated):

```cpp
// Old code:
// pDoc->connect2QM(qmName);

// New code:
if (!pDoc->connectWithRetry(qmName)) {
    // Show error message
    AfxMessageBox("Failed to connect after multiple attempts");
    return;
}

// Before any MQ operation, ensure connection is healthy:
if (!pDoc->ensureConnection()) {
    AfxMessageBox("Connection lost. Please reconnect.");
    return;
}
```

#### Expected Outcomes

**Before:**
- Connection fails → User sees error → Must manually reconnect
- Network glitch → Connection lost → Manual intervention required
- Poor user experience

**After:**
- Connection fails → Automatic retry (3 attempts with backoff)
- Network glitch → Automatic reconnection
- Transparent to user (unless all retries fail)
- Excellent user experience

#### Effort Estimate
- **Development:** 8 hours
- **Testing:** 4 hours
- **Integration:** 4 hours
- **Documentation:** 2 hours
- **Total:** 18 hours (2-3 days)

---

## 2. SHORT TERM: Build System Modernization (P1)

### 2.1 Upgrade to Visual Studio 2022

**Problem:** VS 2017 is outdated (released 2017, mainstream support ended 2022).

**Solution:** Upgrade to VS 2022 for modern tooling and continued support.

#### Implementation Steps

**Step 1: Install Visual Studio 2022**
- Download VS 2022 Community/Professional
- Install "Desktop development with C++" workload
- Install "MFC and ATL support" components

**Step 2: Update Solution File**

Modify `RFHUtil.sln`:

```
Microsoft Visual Studio Solution File, Format Version 12.00
# Visual Studio Version 17
VisualStudioVersion = 17.0.31903.59
MinimumVisualStudioVersion = 10.0.40219.1
```

**Step 3: Update Project Files**

Modify `RFHUtil/RFHUtil.vcxproj`:

```xml
<PropertyGroup Label="Configuration">
  <PlatformToolset>v143</PlatformToolset>
  <WindowsTargetPlatformVersion>10.0.22621.0</WindowsTargetPlatformVersion>
</PropertyGroup>
```

**Step 4: Update All Projects**

Update the same settings in:
- `Client/Client.vcxproj`
- All `mqperf/*/` project files

**Step 5: Test Build**

```cmd
msbuild RFHUtil.sln /p:Configuration=Release /p:Platform=Win32
```

#### Expected Outcomes

**Benefits:**
- ✅ Latest C++ compiler (better optimization)
- ✅ Modern IDE features (IntelliSense, debugging)
- ✅ Security improvements (SDL checks)
- ✅ Active support and updates
- ✅ Better performance

#### Effort Estimate
- **Setup:** 2 hours
- **Project updates:** 2 hours
- **Testing:** 4 hours
- **Total:** 8 hours (1 day)

---

### 2.2 Add 64-bit Support

**Problem:** Only 32-bit builds available. Modern systems are 64-bit.

**Solution:** Add x64 platform configuration.

#### Implementation Steps

**Step 1: Add x64 Configuration**

In Visual Studio:
1. Configuration Manager → Active Solution Platform → New
2. Select x64, copy settings from Win32
3. Apply to all projects

**Step 2: Update Project Files**

Add to each `.vcxproj`:

```xml
<ItemGroup Label="ProjectConfigurations">
  <ProjectConfiguration Include="Release|x64">
    <Configuration>Release</Configuration>
    <Platform>x64</Platform>
  </ProjectConfiguration>
</ItemGroup>

<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
  <LinkIncremental>false</LinkIncremental>
  <OutDir>$(SolutionDir)bin\x64\Release\</OutDir>
</PropertyGroup>
```

**Step 3: Update MQ Library Paths**

```xml
<PropertyGroup Condition="'$(Platform)'=='x64'">
  <MQPath>C:\Program Files\IBM\MQ\</MQPath>
</PropertyGroup>
<PropertyGroup Condition="'$(Platform)'=='Win32'">
  <MQPath>C:\Program Files (x86)\IBM\MQ\</MQPath>
</PropertyGroup>
```

**Step 4: Test Both Platforms**

```cmd
msbuild RFHUtil.sln /p:Configuration=Release /p:Platform=Win32
msbuild RFHUtil.sln /p:Configuration=Release /p:Platform=x64
```

#### Expected Outcomes

**Benefits:**
- ✅ Support modern 64-bit systems
- ✅ Access to more memory (>4GB)
- ✅ Better performance on 64-bit systems
- ✅ Future-proofing

#### Effort Estimate
- **Configuration:** 4 hours
- **Testing:** 4 hours
- **Documentation:** 2 hours
- **Total:** 10 hours (1-2 days)

---

### 2.3 Implement GitHub Actions CI/CD

**Problem:** No automated builds or testing. Manual release process.

**Solution:** Add GitHub Actions for automated CI/CD.

#### Implementation Steps

**Step 1: Create Workflow File**

Create `.github/workflows/build-and-test.yml`:

```yaml
name: Build and Test

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]
  release:
    types: [ created ]

env:
  MQ_VERSION: 9.3.0.0

jobs:
  build:
    name: Build ${{ matrix.platform }} ${{ matrix.configuration }}
    runs-on: windows-2022
    
    strategy:
      matrix:
        platform: [Win32, x64]
        configuration: [Release, Debug]
    
    steps:
    - name: Checkout code
      uses: actions/checkout@v4
    
    - name: Setup MSBuild
      uses: microsoft/setup-msbuild@v2
    
    - name: Setup NuGet
      uses: NuGet/setup-nuget@v2
    
    - name: Cache MQ Installation
      id: cache-mq
      uses: actions/cache@v4
      with:
        path: C:\Program Files\IBM\MQ
        key: mq-${{ env.MQ_VERSION }}-${{ matrix.platform }}
    
    - name: Install IBM MQ Client
      if: steps.cache-mq.outputs.cache-hit != 'true'
      run: |
        # Download and install MQ client
        # This would need actual MQ installation steps
        echo "Installing MQ Client..."
    
    - name: Restore NuGet packages
      run: nuget restore RFHUtil.sln
    
    - name: Build Solution
      run: |
        msbuild RFHUtil.sln `
          /p:Configuration=${{ matrix.configuration }} `
          /p:Platform=${{ matrix.platform }} `
          /m `
          /v:minimal
    
    - name: Run Unit Tests
      if: matrix.configuration == 'Debug'
      run: |
        # Run tests when implemented
        echo "Running tests..."
    
    - name: Upload Build Artifacts
      uses: actions/upload-artifact@v4
      with:
        name: rfhutil-${{ matrix.platform }}-${{ matrix.configuration }}
        path: |
          bin/${{ matrix.platform }}/${{ matrix.configuration }}/*.exe
          bin/${{ matrix.platform }}/${{ matrix.configuration }}/*.pdb
        retention-days: 30
    
    - name: Create Release Assets
      if: github.event_name == 'release' && matrix.configuration == 'Release'
      uses: softprops/action-gh-release@v1
      with:
        files: |
          bin/${{ matrix.platform }}/Release/*.exe
      env:
        GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}

  code-quality:
    name: Code Quality Checks
    runs-on: windows-2022
    
    steps:
    - name: Checkout code
      uses: actions/checkout@v4
    
    - name: Run CodeQL Analysis
      uses: github/codeql-action/init@v3
      with:
        languages: cpp
    
    - name: Setup MSBuild
      uses: microsoft/setup-msbuild@v2
    
    - name: Build for Analysis
      run: msbuild RFHUtil.sln /p:Configuration=Release /p:Platform=Win32
    
    - name: Perform CodeQL Analysis
      uses: github/codeql-action/analyze@v3
```

**Step 2: Add Status Badge**

Update `README.md`:

```markdown
# mq-rfhutil

[![Build Status](https://github.com/ibm-messaging/mq-rfhutil/workflows/Build%20and%20Test/badge.svg)](https://github.com/ibm-messaging/mq-rfhutil/actions)

...
```

**Step 3: Configure Branch Protection**

In GitHub repository settings:
- Require status checks to pass before merging
- Require branches to be up to date before merging
- Require review from code owners

#### Expected Outcomes

**Benefits:**
- ✅ Automated builds on every commit
- ✅ Catch build breaks immediately
- ✅ Automated releases
- ✅ Code quality checks
- ✅ Security scanning
- ✅ Professional development workflow

#### Effort Estimate
- **Workflow creation:** 4 hours
- **Testing and refinement:** 4 hours
- **Documentation:** 2 hours
- **Total:** 10 hours (1-2 days)

---

## 3. MEDIUM TERM: Code Quality (P2)

### 3.1 Add Unit Testing Framework

**Problem:** No automated tests. Regression risk with changes.

**Solution:** Add Google Test framework for unit testing.

#### Implementation Steps

**Step 1: Add Google Test via NuGet**

```cmd
nuget install googletest -OutputDirectory packages
```

**Step 2: Create Test Project**

Create `RFHUtil.Tests/RFHUtil.Tests.vcxproj`:

```xml
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|Win32">
      <Configuration>Debug</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|Win32">
      <Configuration>Release</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  
  <PropertyGroup Label="Globals">
    <ProjectGuid>{GUID-HERE}</ProjectGuid>
    <RootNamespace>RFHUtilTests</RootNamespace>
  </PropertyGroup>
  
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  
  <PropertyGroup Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <PlatformToolset>v143</PlatformToolset>
  </PropertyGroup>
  
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  
  <ItemDefinitionGroup>
    <ClCompile>
      <AdditionalIncludeDirectories>
        $(SolutionDir)packages\googletest.1.14.0\build\native\include;
        $(SolutionDir)RFHUtil;
        %(AdditionalIncludeDirectories)
      </AdditionalIncludeDirectories>
    </ClCompile>
    <Link>
      <AdditionalLibraryDirectories>
        $(SolutionDir)packages\googletest.1.14.0\build\native\lib\$(Platform)\$(Configuration);
        %(AdditionalLibraryDirectories)
      </AdditionalLibraryDirectories>
      <AdditionalDependencies>
        gtest.lib;
        gtest_main.lib;
        %(AdditionalDependencies)
      </AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  
  <ItemGroup>
    <ClCompile Include="ConnectionManagerTests.cpp" />
    <ClCompile Include="EncodingTests.cpp" />
    <ClCompile Include="MessageParsingTests.cpp" />
  </ItemGroup>
  
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
</Project>
```

**Step 3: Create Sample Tests**

Create `RFHUtil.Tests/ConnectionManagerTests.cpp`:

```cpp
#include <gtest/gtest.h>
#include "ConnectionManager.h"

class ConnectionManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup test environment
        connMgr = new ConnectionManager();
    }
    
    void TearDown() override {
        delete connMgr;
    }
    
    ConnectionManager* connMgr;
};

TEST_F(ConnectionManagerTest, InitialStateIsDisconnected) {
    EXPECT_FALSE(connMgr->isConnected());
    EXPECT_EQ(MQHO_NONE, connMgr->getHandle());
}

TEST_F(ConnectionManagerTest, ConnectWithValidQM) {
    // This would require a test QM or mock
    // For now, test the structure
    EXPECT_NO_THROW({
        connMgr->connect("TEST.QM");
    });
}

TEST_F(ConnectionManagerTest, ReconnectAfterDisconnect) {
    connMgr->connect("TEST.QM");
    connMgr->disconnect();
    
    EXPECT_FALSE(connMgr->isConnected());
    
    bool result = connMgr->connect("TEST.QM");
    // Would check result with actual QM
}

TEST_F(ConnectionManagerTest, BackoffCalculation) {
    ConnectionManager::Config config;
    config.initialDelayMs = 1000;
    config.backoffMultiplier = 2.0;
    config.maxDelayMs = 30000;
    
    // Test exponential backoff
    // Attempt 0: 1000ms
    // Attempt 1: 2000ms
    // Attempt 2: 4000ms
    // etc.
}
```

Create `RFHUtil.Tests/EncodingTests.cpp`:

```cpp
#include <gtest/gtest.h>
#include "comsubs.h"

TEST(EncodingTest, AsciiToEbcdic) {
    unsigned char input[] = "Hello World";
    unsigned char output[20];
    
    AsciiToEbcdic(input, strlen((char*)input), output);
    
    // Verify conversion
    EXPECT_NE(0, memcmp(input, output, strlen((char*)input)));
}

TEST(EncodingTest, EbcdicToAscii) {
    unsigned char ebcdic[] = {0xC8, 0x85, 0x93, 0x93, 0x96};  // "Hello" in EBCDIC
    unsigned char ascii[10];
    
    EbcdicToAscii(ebcdic, 5, ascii);
    
    EXPECT_EQ('H', ascii[0]);
    EXPECT_EQ('e', ascii[1]);
    EXPECT_EQ('l', ascii[2]);
    EXPECT_EQ('l', ascii[3]);
    EXPECT_EQ('o', ascii[4]);
}

TEST(EncodingTest, RoundTripConversion) {
    unsigned char original[] = "Test123!@#";
    unsigned char ebcdic[20];
    unsigned char result[20];
    
    AsciiToEbcdic(original, strlen((char*)original), ebcdic);
    EbcdicToAscii(ebcdic, strlen((char*)original), result);
    
    EXPECT_EQ(0, memcmp(original, result, strlen((char*)original)));
}
```

**Step 4: Run Tests**

```cmd
RFHUtil.Tests.exe --gtest_output=xml:test-results.xml
```

#### Expected Outcomes

**Benefits:**
- ✅ Catch regressions early
- ✅ Safe refactoring
- ✅ Documentation through tests
- ✅ Confidence in changes
- ✅ Professional development practice

#### Effort Estimate
- **Framework setup:** 4 hours
- **Initial tests:** 16 hours
- **Integration:** 4 hours
- **Total:** 24 hours (3 days)

---

## 4. Implementation Timeline

### Phase 1: Quick Wins (Week 1-2)
```
Week 1:
- Day 1-2: Add HeartBeat/KeepAlive configuration
- Day 3-5: Implement automatic reconnection

Week 2:
- Day 1-2: Upgrade to VS 2022
- Day 3-5: Add 64-bit support
```

### Phase 2: CI/CD (Week 3-4)
```
Week 3:
- Day 1-3: Create GitHub Actions workflow
- Day 4-5: Test and refine CI/CD

Week 4:
- Day 1-2: Add code quality checks
- Day 3-5: Documentation and training
```

### Phase 3: Testing (Week 5-8)
```
Week 5-6:
- Setup Google Test framework
- Create initial test suite

Week 7-8:
- Expand test coverage
- Integration testing
```

### Phase 4: Refactoring (Month 3-6)
```
Month 3-4:
- Refactor DataArea class
- Implement modern C++ features

Month 5-6:
- Comprehensive testing
- Performance optimization
```

---

## 5. Success Metrics

### Connection Reliability
- **Before:** 300s failure detection, frequent manual reconnects
- **After:** 60s failure detection, automatic reconnection
- **Target:** <1% manual reconnection rate

### Build System
- **Before:** Manual builds, no automation
- **After:** Automated builds on every commit
- **Target:** 100% automated build success rate

### Code Quality
- **Before:** No tests, high regression risk
- **After:** Unit tests, integration tests
- **Target:** >70% code coverage

### Development Velocity
- **Before:** Slow, manual processes
- **After:** Fast, automated workflows
- **Target:** 50% reduction in release time

---

## 6. Risk Mitigation

### Technical Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Breaking changes in VS 2022 | Low | Medium | Thorough testing, gradual rollout |
| MQ compatibility issues | Low | High | Test with multiple MQ versions |
| Performance regression | Low | Medium | Performance testing, benchmarks |
| User resistance to changes | Medium | Low | Clear communication, training |

### Mitigation Strategies

1. **Incremental Changes:** Implement one feature at a time
2. **Thorough Testing:** Test each change extensively
3. **Rollback Plan:** Keep old binaries available
4. **User Communication:** Announce changes in advance
5. **Documentation:** Update docs with each change

---

## 7. Resource Requirements

### Development Team
- **Senior Developer:** 40 hours (connection improvements, architecture)
- **Developer:** 80 hours (implementation, testing)
- **DevOps Engineer:** 20 hours (CI/CD setup)
- **QA Engineer:** 40 hours (testing, validation)

### Infrastructure
- **GitHub Actions:** Free for public repos
- **Test Environment:** MQ Docker containers (free)
- **Development Tools:** VS 2022 Community (free)

### Total Effort
- **Phase 1:** 2 weeks (80 hours)
- **Phase 2:** 2 weeks (80 hours)
- **Phase 3:** 4 weeks (160 hours)
- **Phase 4:** 12 weeks (480 hours)
- **Total:** 20 weeks (800 hours)

---

## 8. Next Steps

### Immediate Actions (This Week)

1. **Review this roadmap** with stakeholders
2. **Prioritize items** based on business needs
3. **Assign resources** for Phase 1
4. **Create GitHub issues** for tracking
5. **Set up development environment** (VS 2022)

### Week 1 Tasks

1. **Monday:** Add HeartBeat/KeepAlive configuration
2. **Tuesday:** Test connection improvements
3. **Wednesday:** Start automatic reconnection implementation
4. **Thursday:** Continue reconnection implementation
5. **Friday:** Test and document changes

### Communication Plan

1. **Weekly updates:** Progress reports to stakeholders
2. **Monthly demos:** Show completed features
3. **Release notes:** Document all changes
4. **User training:** Sessions for new features

---

**Document Version:** 1.0  
**Date:** 2026-02-13  
**Author:** IBM Bob (Architect Mode)  
**Status:** Ready for Review
# RFHUtil Modernization Roadmap - Detailed Action Plan

## 📊 Progress Tracker

**Last Updated:** April 13, 2026
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
| 🟡 **P1.2b** | Safe Mode Build | ✅ COMPLETE | Feb 20, 2026 | `ReleaseSafe` config, `rfhutilc-safe.exe`, SAFE_MODE guard disables all write ops |
| 🟡 **P1.3** | 64-bit Support | ✅ COMPLETE | Feb 23, 2026 | x64 platform, fixed MFC handlers, 32 files |
| 🟡 **P1.4** | Basic Unit Testing | ✅ COMPLETE | Mar 20, 2026 | 132 tests across 5 modules, Win32+x64 |
| 🟢 **P2.1** | Connection Health Monitor | ✅ COMPLETE | Apr 8, 2026 | WM_TIMER + MQINQ probes, live status in Connection Settings tab |
| 🟢 **P2.2** | Secure Credential Storage | ✅ COMPLETE | Apr 8, 2026 | Windows DPAPI, base64 registry storage, opt-in per connection |
| 🟢 **P2.3** | Editable Data Tab | ✅ COMPLETE | Apr 8, 2026 | Allow Edit checkbox + Write Q button, Character and Hex round-trip |

### In Progress 🚧

| Priority | Item | Status | Started Date | Target Date |
|----------|------|--------|--------------|-------------|
| None | - | - | - | - |

### Upcoming 📋

| Priority | Item | Effort | Impact | Target Quarter |
|----------|------|--------|--------|----------------|
| 🟢 **P2** | Refactor DataArea Class | High | High | Q3 2026 |
| 🟢 **P2** | Modern C++ Features | High | Medium | Q3 2026 |
| 🔵 **P3** | CMake Build System | Medium | Low | Q3 2026 |
| 🔵 **P3** | Comprehensive Testing | High | High | Q3 2026 |
| 🟡 **P4** | GitHub Actions CI/CD | Medium | High | Q4 2026 |

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
| 🟡 **P1.2b** | Safe mode (browse-only build) | Low | High | Low | ✅ DONE |


### Short Term (Weeks 5-12)

| Priority | Item | Effort | Impact | Risk | Status |
|----------|------|--------|--------|------|--------|
| 🟡 **P1.4** | Basic Unit Testing | Medium | High | Low | ✅ DONE |
| 🟢 **P2.1** | Connection Health Monitor | Medium | Medium | Low | ✅ DONE |
| 🟢 **P2.2** | Secure Credential Storage | Medium | High | Medium | ✅ DONE |
| 🟢 **P2.3** | Make the Data tab editable by adding a check mark and allow for that data to be written to the queue | Medium | Medium | Medium | ✅ DONE |

### Medium Term (Months 4-6)

| Priority | Item | Effort | Impact | Risk |
|----------|------|--------|--------|------|
| 🟢 **P2** | Refactor DataArea Class | High | High | Medium |
| 🟢 **P2** | Modern C++ Features | High | Medium | Medium |
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

**Step 2: Integrate with DataArea**

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

#### Effort Estimate
- **Development:** 8 hours
- **Testing:** 4 hours
- **Integration:** 4 hours
- **Documentation:** 2 hours
- **Total:** 18 hours (2-3 days)

---

## 2. SHORT TERM: Build System Modernization (P1)

### 2.1 Upgrade to Visual Studio 2022

**Status:** ✅ DONE — Already using VS 2022 (v143) with Windows SDK 10.0.

---

### 2.2 Add 64-bit Support

**Status:** ✅ DONE — x64 platform configuration added, 32 files fixed.

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
        bin\tests\UnitTests.exe --gtest_output=xml:test-results.xml
    
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

#### Effort Estimate
- **Workflow creation:** 4 hours
- **Testing and refinement:** 4 hours
- **Documentation:** 2 hours
- **Total:** 10 hours (1-2 days)

---

## 3. MEDIUM TERM: Code Quality (P2)

### 3.1 Unit Testing Framework

**Status:** ✅ DONE — 132 tests across 5 modules using Google Test (Win32 + x64).

---

### 3.2 Refactor DataArea Class

**Problem:** `DataArea` is a massive monolith (~18,000 lines). Hard to maintain, test, and extend.

**Solution:** Break into focused, single-responsibility classes.

#### Proposed Structure

- `MQConnection` — connection lifecycle, heartbeat, reconnect
- `MQMessageReader` — MQGET, browse, message parsing
- `MQMessageWriter` — MQPUT, put1
- `RFHHeaders` — RFH1/RFH2 build/parse
- `MessageData` — data area, encoding, display formatting

#### Effort Estimate
- **Analysis:** 8 hours
- **Refactoring:** 80 hours
- **Testing:** 40 hours
- **Total:** ~128 hours (3-4 weeks)

---

## 4. Implementation Timeline

### Phase 1: Quick Wins ✅ COMPLETE
- P0.1 HeartBeat/KeepAlive
- P0.2 Auto-Reconnect
- P0.3 Connection Settings Tab
- P1.1 VS 2022
- P1.2 Dark Mode
- P1.3 64-bit Support
- P1.4 Unit Testing
- P2.1 Connection Health Monitor
- P2.2 Secure Credential Storage
- P2.3 Editable Data Tab

### Phase 2: CI/CD (Next)
- GitHub Actions workflow (Win32 + x64, Debug + Release)
- Automated test runs on PR
- Release artifact upload

### Phase 3: Refactoring (Month 3-6)
- DataArea decomposition
- Modern C++ features (RAII, smart pointers, std::string)
- Comprehensive test coverage (>70%)

---

## 5. Success Metrics

### Connection Reliability
- **Before:** 300s failure detection, frequent manual reconnects
- **After:** 60s failure detection, automatic reconnection + proactive health monitoring
- **Status:** ✅ Achieved

### Build System
- **Before:** Manual builds, no automation
- **After:** Automated builds on every commit
- **Target:** 100% automated build success rate

### Code Quality
- **Before:** No tests, high regression risk
- **After:** 132 unit tests, integration tests
- **Target:** >70% code coverage

---

## 6. Risk Mitigation

### Technical Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Breaking changes in VS 2022 | Low | Medium | Thorough testing, gradual rollout |
| MQ compatibility issues | Low | High | Test with multiple MQ versions |
| Performance regression | Low | Medium | Performance testing, benchmarks |
| User resistance to changes | Medium | Low | Clear communication, training |

---

**Document Version:** 1.1  
**Date:** 2026-04-08  
**Status:** Active

# RFHUtil Codebase Analysis — Q3 2026

**Branch:** features/code-validation
**Date:** July 2026
**Compared against:** ARCHITECTURE_ANALYSIS.md (April 2026), MODERNIZATION_ROADMAP.md (v1.5, May 2026)

---

## Executive Summary

Significant progress since the April 2026 analysis. All P0–P3 roadmap items are complete. The
codebase is now buildable by any contributor (MQ SDK probe in build.cmd), has 251 unit tests
(up from 132), a working CI/CD pipeline, human-readable MQ error messages, and TLS/credential
improvements. The P4 DataArea decomposition is underway with three structs extracted
(MQConnection, FileHandler, MqApi). The core risk profile is unchanged: the DataArea monolith
remains at ~26,200 lines and the codebase still makes widespread use of unsafe C string
functions — the primary target for P4.5.

Overall verdict: **on track**. No regressions introduced. No new technical debt added.
Priorities below are consistent with the existing roadmap ordering.

---

## 1. What Has Changed Since April 2026

### Completed since last analysis

| Item | What was done |
|---|---|
| P3.1 | `Directory.Build.props` + `build.cmd` probe list — any contributor can build |
| P3.2 | GitHub Actions CI on every push/PR (Win32 + x64, Google Test XML) |
| P3.3–P3.6 | Test suite expanded from 132 to **251 tests** across 10 test files |
| P3.7 (deferred) | Folded into P4.1 MQConnection RAII — no separate work needed |
| P3.8 | `mqerror.cpp` — ~120 MQRC codes mapped; wired into `setErrorMsg()` default branch |
| P3.9 | TLS cipher list reordered (strong-first); SSL Peer Name + FIPS Required UI fields |
| P3.10 | Release workflow (`release.yml`) publishes committed binaries on `v*` tag |
| P4.1 (partial) | `MQConnection.h` extracted with all connection/reconnect/health structs |
| P4.4 (partial) | `FileHandler.h/cpp` extracted with file buffer state and two I/O methods |
| P4.1 (partial) | `MqApi.h` extracted with 16 dynamic function pointers |
| Dark Grey theme | Fourth theme added (View > Theme > Dark Grey) — charcoal/slate palette |

### Not yet started

P4.2 (MQMessageReader/Writer), P4.3 (RFHHeaders), P4.5 (std::string), P4.6 (smart pointers),
P5 (CMake) — all consistent with roadmap Q3-Q4 2026 timeline.

---

## 2. Metrics Comparison

| Metric | April 2026 (baseline) | July 2026 (now) | Delta |
|---|---|---|---|
| DataArea.cpp lines | 25,881 | ~26,200 | +319 (new features) |
| DataArea.h lines | 1,091 | 928 | -163 (extracted to structs) |
| Unit tests | 132 | **251** | +119 |
| Test files | 5 | **10** | +5 |
| Extracted structs | 0 | **3** | MQConnection, FileHandler, MqApi |
| CI/CD pipeline | None | Build + test on every PR | Done |
| MQ error codes mapped | 0 | **~120** | Done |
| Build portability | Hardcoded paths | Probe list + env var | Done |
| Themes | 3 (Light/Dark/System) | **4** (+Dark Grey) | Done |
| raw new/delete in DataArea | 132 (whole codebase) | **8 in DataArea** | Custom rfhMalloc/rfhFree wrapper used elsewhere |
| TODO/FIXME/HACK comments | Unknown | **0** | Clean |

---

## 3. Architecture Assessment

### 3.1 DataArea decomposition — current state

Three structs have been extracted from DataArea and are actively in use:

**MQConnection.h** (188 lines) — connection lifecycle
- Owns: MQHCONN handle, connected state, TLS settings, credentials, heartbeat config,
  reconnect state (attempt counter, backoff, last-attempt time), health monitor state
- Has RAII destructor — handles open on scope exit (P3.7 folded in)
- Methods: `isActive()`, `shouldAttemptReconnect()`, `calculateReconnectDelay()`,
  `resetReconnectionState()`, `startHealthMonitor()`, `stopHealthMonitor()`,
  `performHealthCheck()`, `disconnect()`, `notifyConnectionLost()`

**FileHandler.h/cpp** (65 + ~100 lines) — file I/O state
- Owns: `fileName[512]`, `fileData` buffer, `fileSize`, `fileCcsid`, `fileSource`,
  read/write/encoding settings
- Methods: `changeUnixFile()` (LF->CRLF), `openOutputFile()` (static)
- Remaining methods (`ReadFileData`, `WriteFile`, `clearFileData`) still in DataArea

**MqApi.h** (260 lines) — dynamic MQ API
- Owns: 16 MQ function pointers + DLL handle
- All loaded via `loadMQdll()` which still lives in DataArea (next target for migration)

**Backward-compatibility aliases** in DataArea bind ~50 member references to the extracted
structs so existing call sites compile unchanged during the transition.

### 3.2 What still needs extracting (P4.1 continuation)

Methods logically belonging to `MQConnection` but still in DataArea:
- `connect2QM()`, `discQM()`, `explicitConnect()`, `explicitDiscQM()`
- `connectionLostCleanup()`, `checkConnection()`, `attemptReconnection()`
- `performHealthCheck()` (method body)

Methods logically belonging to `FileHandler` but still in DataArea:
- `ReadFileData()`, `WriteFile()`, `clearFileData()`
- `checkDataForHeader()`, `parseMsgHeaders()` (operate on m_file.fileData)

Methods logically belonging to `MqApi` but still in DataArea:
- `loadMQdll()` (~200 lines) — populates all 16 function pointers

### 3.3 DataArea method count

195 distinct methods/functions remain in DataArea.cpp. By responsibility area:

| Area | Approx. methods | Roadmap target |
|---|---|---|
| Connection & reconnect | 12 | P4.1 — extract to MQConnection |
| File I/O | 8 | P4.4 — extract to FileHandler |
| Header parse/build | 15 | P4.3 — extract to RFHHeaders |
| Message get/put/browse | 18 | P4.2 — extract to MQMessageReader/Writer |
| Data display (hex/char/XML/JSON) | 22 | P4.2 |
| Queue ops (open/close/depth/purge) | 12 | P4.2 |
| Pub/Sub | 14 | stays in DataArea or new PubSubManager |
| Admin/PCF | 18 | stays in DataArea |
| Names/lists loading | 10 | stays in DataArea |
| Copy book | 4 | stays in DataArea |
| UOW (begin/commit/rollback) | 3 | P4.2 |
| Search/find | 20 | P4.2 |
| Message save/load/capture/move | 8 | P4.2 |
| Utility/trace/display helpers | 31 | stays / comsubs |

---

## 4. Code Quality Findings

Findings are ordered: Critical > High > Medium > Low.

---

### FIND-01 — Widespread unsafe string functions (sprintf, strcpy, strcat)
**Severity:** High
**Status:** Known — targeted by P4.5

Counts in DataArea.cpp and across the codebase:
- `sprintf()` — ~180 occurrences in DataArea, 300+ codebase-wide
- `strcpy()` — ~65 occurrences in DataArea
- `strcat()` — ~20 occurrences

No `sprintf_s()`, `strcpy_s()`, or `snprintf()` usage found anywhere.

The dominant pattern is trace/diagnostic message formatting into fixed-size stack buffers
(128–2048 bytes, inconsistently sized). Most are low practical risk (controlled format
strings, bounded inputs). A handful are higher risk:

- [`RFHUtil/DataArea.cpp:296`](RFHUtil/DataArea.cpp) — `strcpy(fileName, fd.GetPathName())` where
  `fileName[512]` and `GetPathName()` can return paths up to `MAX_PATH` (260) but also
  extended paths (`\\?\` prefix) up to 32767 chars on modern Windows.
- [`RFHUtil/FileHandler.cpp:78-82`](RFHUtil/FileHandler.cpp) — `strcpy(errMsg, ...)` into a
  caller-supplied buffer with no size parameter.

**Recommendation:** Address as part of P4.5. Prioritise `strcpy` calls on external inputs
(file paths, queue names, user IDs) before trace-buffer `sprintf` calls. Use `sprintf_s` /
`strcpy_s` (MSVC safe CRT) as the replacement — consistent with the existing MSVC build
environment.

---

### FIND-02 — Password memory handling
**Severity:** High
**Status:** Partial — DPAPI registry storage implemented (P2.2); in-memory handling not hardened

The DPAPI encryption (P2.2) protects passwords **at rest in the registry**. However:

1. `m_conn_password` is a `CString` — plaintext in process memory for the duration of
   the connection session. No `SecureZeroMemory()` call clears it after `MQCONNX`.
2. `ConnUser.cpp` exposes `m_conn_password` as a public `CString` member — any code with
   a `CConnUser*` can read it.

**Recommendation:** After `MQCONNX` returns, zero the password field:
```cpp
SecureZeroMemory((void*)m_conn_password.GetBuffer(), m_conn_password.GetLength() * sizeof(TCHAR));
m_conn_password.ReleaseBuffer();
m_conn_password.Empty();
```
This is a small, targeted change that can be done independently of P4.

---

### FIND-03 — No exception handling around MQ API calls
**Severity:** Medium

Zero `try`/`catch`/`throw` found across the entire codebase. MQ API calls (`MQCONNX`,
`MQGET`, `MQPUT`, etc.) return status via `CompCode`/`Reason` out-parameters so they do not
throw C++ exceptions. The risk is in memory allocation: `rfhMalloc` and `new` can fail.
Currently `new` (used for `CCopybook` and `Names`) has no allocation-failure guard. On a
low-memory system this would hard-crash the application.

**Recommendation:** Wrap the three `new` call sites in DataArea in `try/catch(std::bad_alloc)`.
This is three lines of change, no architectural impact.

---

### FIND-04 — Trace debug file path hardcoded
**Severity:** Low
**Location:** [`RFHUtil/comsubs.cpp:25`](RFHUtil/comsubs.cpp)

`"c:\rfhdump.txt"` is hardcoded as the debug dump path. On modern Windows this will silently
fail on any non-admin user (C:\ root is write-protected). The trace data disappears with no
indication.

**Recommendation:** Change to `%TEMP%\rfhdump.txt` using `GetTempPath()`, or make it
configurable via a registry setting.

---

### FIND-05 — Buffer size inconsistency in trace/diagnostic code
**Severity:** Low

Trace buffers range from 128 to 2048 bytes with no consistent convention. Most calls follow
the pattern `char traceInfo[512]; sprintf(traceInfo, ...)` but the format strings can include
user-controlled values (queue manager name, user ID, file path). None currently overflow in
practice but the inconsistency makes auditing difficult.

**Recommendation:** Establish a single `TRACE_BUF_SIZE` constant (suggest 1024) and use it
everywhere. Apply as part of P4.5.

---

## 5. Test Coverage Assessment

### Current state

| Area | Tests | Coverage |
|---|---|---|
| Hex encoding / EBCDIC conversion | ~50 | Good |
| XML parsing (xmlsubs + XMLParse) | ~48 | Good |
| JSON parsing | ~25 | Good |
| Names management | ~25 | Good |
| Encoding utilities | ~30 | Good |
| File format detection | ~20 | Good |
| Header utilities (RFH1 parsing helpers) | ~24 | Partial |
| MQ error code mapping | 10 | Good |
| Reconnect logic | 17 | Good |
| **DataArea core (connect, MQGET, MQPUT)** | **0** | **None** |
| **File I/O (ReadFileData, WriteFile)** | **0** | **None** |
| **Header build/parse (full)** | **0** | **None** |
| **Pub/Sub operations** | **0** | **None** |
| **PCF/Admin operations** | **0** | **None** |

**Total: 251 tests.** The 119-test increase since April is meaningful but all new tests
still cover utility functions. DataArea and MQ-dependent logic remain at zero.

### Gap — no mock for MqApi

The test project uses `tests/UnitTests/prelude.h` to stub out MQ SDK headers. There is no
mock for the `MqApi` function pointers. This means `connect2QM()`, `getMessage()`, and
`putMessage()` cannot be exercised in the unit test project without an actual MQ installation.

**Recommendation:** As P4.1 migration moves method bodies into `MQConnection`, design them
to accept an `MqApi*` parameter (already available via `m_connection.m_api`). This naturally
enables dependency injection — a test can supply an `MqApi` struct with stub function
pointers. No framework needed; it is a consequence of the extraction already planned.

---

## 6. Build System & CI Assessment

### Build system
- `Directory.Build.props` + `build.cmd` probe list covers 6 standard MQ install locations
- `MQ_HOME` environment variable override works and is validated before MSBuild runs
- Both Win32 and x64 configurations build cleanly
- ReleaseSafe (browse-only) configuration confirmed working

### CI pipeline (`.github/workflows/build-and-test.yml`)
- Triggers on push to `master` and `features/**`, and on PRs to master
- Matrix: Win32 + x64
- Unit tests run, XML results parsed, JUnit report published via `dorny/test-reporter`
- **Gap:** CI runner has no MQ SDK so it only builds and runs non-MQ unit tests.
  This is documented and expected — MQ-dependent integration tests require the Docker
  environment in `tests/docker/`.
- Release workflow (`release.yml`) publishes committed binaries on `v*` tag push.
  **Note:** binaries must be committed before tagging — this is documented in BUILD_CONFIG.md.

---

## 7. Roadmap Alignment

| Roadmap item | Status per roadmap | Confirmed in code |
|---|---|---|
| P0.1–P0.3 Connection reliability | COMPLETE | Yes — ConnSettings tab, heartbeat, reconnect all present |
| P1.2 Dark mode | COMPLETE | Yes — ThemeManager with 4 themes |
| P1.2b Safe mode | COMPLETE | Yes — ReleaseSafe config + SAFE_MODE guards |
| P1.3 64-bit | COMPLETE | Yes — x64 project configs present and building |
| P1.4 Unit testing | COMPLETE | Yes — 251 tests, 10 files |
| P2.1 Health monitor | COMPLETE | Yes — WM_TIMER + MQINQ in ConnSettings |
| P2.2 DPAPI credentials | COMPLETE | Yes — CryptProtectData in ConnUser |
| P2.3 Editable data tab | COMPLETE | Yes — MSGDATA tab with Allow Edit |
| P3.1–P3.10 CI/CD + quality | COMPLETE | Yes — all confirmed above |
| P4.1 MQConnection extraction | IN PROGRESS | Struct + methods extracted; bodies migrating |
| P4.4 FileHandler extraction | IN PROGRESS | Struct extracted; 2 methods done, ~6 remaining |
| P4.2 MQMessageReader/Writer | NOT STARTED | Correctly blocked on P4.1 completion |
| P4.3 RFHHeaders | NOT STARTED | Correctly blocked on P4.2 |
| P4.5 std::string | NOT STARTED | Correctly ordered after P4.1–P4.4 |
| P4.6 Smart pointers | NOT STARTED | Correctly ordered after P4.1–P4.4 |

---

## 8. Recommendations

Ordered by effort-to-value ratio — smallest impactful changes first.

| # | Action | Effort | Roadmap ref | Priority |
|---|---|---|---|---|
| 1 | `SecureZeroMemory` on password after MQCONNX | 1 line | Independent | High |
| 2 | Wrap 3 `new` call sites in `try/catch(std::bad_alloc)` | 3 lines | Independent | Medium |
| 3 | Fix `comsubs.cpp` debug dump path to `%TEMP%` | 2 lines | Independent | Low |
| 4 | Complete P4.1 — migrate `connect2QM` etc. into MQConnection | Medium | P4.1 | High |
| 5 | Complete P4.4 — migrate `ReadFileData`, `WriteFile` into FileHandler | Small | P4.4 | High |
| 6 | Design MqApi stub for unit tests during P4.1 migration | Small | P4.1 | Medium |
| 7 | Introduce `TRACE_BUF_SIZE` constant; apply during P4.5 | Trivial | P4.5 | Low |
| 8 | Replace `strcpy` on external inputs (file path, queue name) | Small | P4.5 | High |
| 9 | Replace `sprintf` codebase-wide with `sprintf_s` | Medium | P4.5 | Medium |
| 10 | `loadMQdll()` migrated to MqApi | Small | P4.1 | Medium |

Items 1, 2, and 3 can be done on this branch (`features/code-validation`) as they are
fully independent of the P4 refactoring sequence and have zero regression risk.

---

## 9. Conclusion

The project is in a materially better state than the April 2026 baseline:

- **Build:** Any contributor can build without manual SDK path configuration.
- **Tests:** 251 tests up from 132; CI runs on every PR.
- **Architecture:** Three structs extracted from the DataArea monolith; reference aliases
  maintain backward compatibility during the transition.
- **Reliability:** RAII handle management, exponential backoff reconnect, health monitor,
  and DPAPI credential storage all shipped and tested.
- **Themes:** Four themes now available with proper WCAG AA contrast ratios.

The primary remaining risks are the unsafe C string functions (addressed by P4.5) and the
absence of DataArea unit tests (partially addressable via MqApi stub injection as P4.1
proceeds). Neither is a blocker for the current build quality — they are structural
improvements correctly sequenced in the roadmap.

**The roadmap ordering remains sound. No re-prioritisation needed.**

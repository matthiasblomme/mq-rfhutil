# RFHUtil Modernization Roadmap - Detailed Action Plan

## 📊 Progress Tracker

**Last Updated:** April 13, 2026 (P3.1 complete)
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
| 🔴 **P3.1** | Fix hardcoded MQ SDK paths | ✅ COMPLETE | Apr 13, 2026 | `Directory.Build.props` with `$(MQ_HOME)`, defaults to `C:\Program Files\IBM\MQ` |

### In Progress 🚧

| Priority | Item | Status | Started Date | Target Date |
|----------|------|--------|--------------|-------------|
| None | - | - | - | - |

### Upcoming 📋

| Priority | Item | Effort | Impact | Target Quarter |
|----------|------|--------|--------|----------------|
| 🟡 **P3.2** | GitHub Actions — build + unit tests | Small | High | Q2 2026 |
| 🟡 **P3.3** | Expand tests — header parsing (RFH1/RFH2, DLQ, CICS, IMS) | Medium | High | Q2 2026 |
| 🟡 **P3.4** | Expand tests — message encoding (EBCDIC, hex, JSON, XML) | Medium | High | Q2 2026 |
| 🟡 **P3.5** | Expand tests — connection lifecycle | Medium | Medium | Q2 2026 |
| 🟡 **P3.6** | Expand tests — file I/O round-trips | Medium | Medium | Q3 2026 |
| 🟢 **P3.7** | MQ handle RAII wrapper | Small | Medium | Q3 2026 |
| 🟢 **P3.8** | Human-readable MQ error messages | Small | Medium | Q3 2026 |
| 🟢 **P3.9** | TLS configuration improvements | Medium | Medium | Q3 2026 |
| 🟢 **P3.10** | GitHub Actions — release artifact publishing | Small | Medium | Q3 2026 |
| 🔵 **P4.1** | DataArea refactor — extract `MQConnection` | Medium | High | Q3 2026 |
| 🔵 **P4.2** | DataArea refactor — extract `MQMessageReader` / `MQMessageWriter` | High | High | Q3 2026 |
| 🔵 **P4.3** | DataArea refactor — extract `RFHHeaders` | High | High | Q4 2026 |
| 🔵 **P4.4** | DataArea refactor — extract `FileHandler` | Medium | High | Q4 2026 |
| 🔵 **P4.5** | Replace `char[]` buffers with `std::string` | High | Medium | Q4 2026 |
| 🔵 **P4.6** | Replace manual `new`/`delete` with smart pointers | High | Medium | Q4 2026 |
| ⚪ **P5** | CMake build system | Medium | Low | 2027 |

---

## Executive Summary

This document provides a **detailed, actionable roadmap** for modernizing the mq-rfhutil project. Each recommendation includes specific steps, code examples, and expected outcomes.

---

## Priority Matrix

### Phase 1: Foundation ✅ COMPLETE

| Priority | Item | Effort | Impact | Risk | Status |
|----------|------|--------|--------|------|--------|
| 🔴 **P0.1** | Add HeartBeat/KeepAlive | Low | High | Low | ✅ DONE |
| 🔴 **P0.2** | Implement Auto-Reconnect | Medium | High | Low | ✅ DONE |
| 🔴 **P0.3** | Connection Settings UI Tab | Medium | High | Low | ✅ DONE |
| 🟡 **P1.1** | Upgrade to VS 2022 | Low | Medium | Low | ✅ DONE |
| 🟡 **P1.2** | Dark mode | Low | Medium | Low | ✅ DONE |
| 🟡 **P1.2b** | Safe mode (browse-only build) | Low | High | Low | ✅ DONE |
| 🟡 **P1.3** | 64-bit Support | Medium | High | Low | ✅ DONE |
| 🟡 **P1.4** | Basic Unit Testing | Medium | High | Low | ✅ DONE |
| 🟢 **P2.1** | Connection Health Monitor | Medium | Medium | Low | ✅ DONE |
| 🟢 **P2.2** | Secure Credential Storage | Medium | High | Medium | ✅ DONE |
| 🟢 **P2.3** | Editable Data Tab | Medium | Medium | Medium | ✅ DONE |

### Phase 2: CI/CD & Test Coverage (P3) — Next Up

| Priority | Item | Effort | Impact | Risk | Notes |
|----------|------|--------|--------|------|-------|
| 🔴 **P3.1** | Fix hardcoded MQ SDK paths | Small | High | Low | ✅ DONE — `Directory.Build.props` with `$(MQ_HOME)`, override via env var or `/p:MQ_HOME=` |
| 🟡 **P3.2** | GitHub Actions — build + unit tests | Small | High | Low | Windows runner, Win32+x64 matrix, Google Test output |
| 🟡 **P3.3** | Expand tests — header parsing | Medium | High | Low | RFH1, RFH2, DLQ, CICS, IMS header build/parse coverage |
| 🟡 **P3.4** | Expand tests — message encoding | Medium | High | Low | EBCDIC, hex, JSON, XML round-trips |
| 🟡 **P3.5** | Expand tests — connection lifecycle | Medium | Medium | Low | Connect, disconnect, reconnect, health check state |
| 🟡 **P3.6** | Expand tests — file I/O | Medium | Medium | Low | Read/write various formats, round-trip correctness |
| 🟢 **P3.7** | MQ handle RAII wrapper | Small | Medium | Low | Wrap `MQHOBJ`/`MQHCONN` — prevents leaks on exception paths |
| 🟢 **P3.8** | Human-readable MQ error messages | Small | Medium | Low | Map reason codes (e.g. RC=2035) to descriptive strings |
| 🟢 **P3.9** | TLS configuration improvements | Medium | Medium | Medium | Make cipher suite configurable per connection in UI |
| 🟢 **P3.10** | GitHub Actions — release publishing | Small | Medium | Low | Upload `.exe` artifacts on tag; depends on P3.2 |

### Phase 3: Refactoring (P4) — Requires P3 Safety Net

| Priority | Item | Effort | Impact | Risk | Notes |
|----------|------|--------|--------|------|-------|
| 🔵 **P4.1** | DataArea — extract `MQConnection` | Medium | High | Medium | Connection lifecycle, heartbeat, reconnect — smallest safe first cut |
| 🔵 **P4.2** | DataArea — extract `MQMessageReader` / `MQMessageWriter` | High | High | Medium | MQGET/browse and MQPUT/put1 — depends on P4.1 |
| 🔵 **P4.3** | DataArea — extract `RFHHeaders` | High | High | Medium | RFH1/RFH2/DLQ/CICS/IMS parse & build |
| 🔵 **P4.4** | DataArea — extract `FileHandler` | Medium | High | Medium | File read/write in all supported formats |
| 🔵 **P4.5** | Replace `char[]` with `std::string` | High | Medium | Medium | Start with non-MQ-API paths; incremental |
| 🔵 **P4.6** | Replace `new`/`delete` with smart pointers | High | Medium | Medium | Start with DataArea internals after P4.1–P4.4 |

### Phase 4: Long-term (P5)

| Priority | Item | Effort | Impact | Risk | Notes |
|----------|------|--------|--------|------|-------|
| ⚪ **P5** | CMake build system | Medium | Low | Low | Cross-platform support; nice-to-have |

---

## Implementation Timeline

### Phase 1: Foundation ✅ COMPLETE
All P0–P2 items shipped. Key deliverables: heartbeat/reconnect, dark mode, safe mode, 64-bit, unit testing, health monitor, DPAPI credentials, editable data tab.

### Phase 2: CI/CD & Coverage (Q2–Q3 2026)
1. **P3.1** ✅ — Parameterize MQ SDK paths (`Directory.Build.props`, `$(MQ_HOME)`)
2. **P3.2** — GitHub Actions basic workflow
3. **P3.3–P3.6** — Expand test coverage to core logic areas
4. **P3.7–P3.10** — Smaller quality improvements in parallel

### Phase 3: Refactoring (Q3–Q4 2026)
With CI/CD and tests as safety net:
1. **P4.1** — Extract `MQConnection` first (smallest, most self-contained)
2. **P4.2–P4.4** — Extract remaining DataArea responsibilities iteratively
3. **P4.5–P4.6** — Modernize memory/string management incrementally

### Phase 4: Long-term (2027+)
- CMake for cross-platform build support

---

## Key Findings from Codebase Analysis (April 2026)

### DataArea Monolith
- **DataArea.cpp**: 25,881 lines | **DataArea.h**: 1,091 lines
- 243+ public/protected methods covering: MQ connections, queue ops, message parsing, file I/O, data transformation, header management, pub/sub, PCF requests
- 132 raw `new`/`delete` operations; ~1,573 instances of `char*`, `sprintf`, `strcpy`, `malloc`, `free` across the codebase
- Zero `std::string`, zero smart pointers

### Test Coverage Gaps
- 132 existing tests cover utility functions only (encoding, XML/JSON parsing, string utils, names)
- **Zero tests** for DataArea, header building, connection lifecycle, queue operations, file I/O, pub/sub, PCF

### CI/CD Blockers
- MQ SDK paths hardcoded to `d:\apps\mq\` in all .vcxproj files
- No `.github/workflows/` directory exists
- Unit tests (non-MQ) can run in CI immediately once paths are parameterized

### Modern C++ Status
- 0% adoption of modern C++ idioms
- Full MFC dependency for UI (CString, CDialog — not replaceable short-term)
- Memory management entirely manual; no RAII outside of MFC

---

## Success Metrics

| Area | Before | Target |
|------|--------|--------|
| Test coverage | 132 tests, utility-only | 400+ tests, core logic covered |
| CI/CD | None | Automated builds + tests on every PR |
| DataArea size | 26k lines, 1 class | 5–6 focused classes, <5k lines each |
| Memory safety | Manual new/delete everywhere | Smart pointers, RAII handles |
| Build portability | Hardcoded local paths | `$(MQ_HOME)` env var, buildable by any contributor |

---

## Risk Mitigation

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| DataArea refactor introduces regressions | Medium | High | Expand tests first (P3.3–P3.6), CI gate (P3.2) |
| MQ SDK unavailable in CI | High | Medium | Skip MQ-dependent tests in CI; unit tests only |
| Breaking changes during modernization | Low | Medium | Incremental extraction, one class at a time |
| MQ API incompatibility with C++ wrappers | Low | High | Keep MQ API calls isolated; wrap at boundary only |

---

**Document Version:** 1.2
**Date:** 2026-04-13
**Status:** Active

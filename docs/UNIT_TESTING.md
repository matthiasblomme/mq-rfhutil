# Unit Testing for mq-rfhutil

## Overview

The unit test suite validates core parsing, encoding, and utility functions using
Google Test (gtest) v1.8.1.8 via NuGet.  Tests run as a `/SUBSYSTEM:WINDOWS` MFC
application — there is no console stdout, so results are captured via
`--gtest_output=xml:<path>`.

## Quick Start

### Build

```bash
# Win32 Debug
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" ^
  tests\UnitTests\UnitTests.vcxproj -t:Build -p:Configuration=Debug -p:Platform=Win32 -v:minimal

# x64 Debug
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" ^
  tests\UnitTests\UnitTests.vcxproj -t:Build -p:Configuration=Debug -p:Platform=x64 -v:minimal
```

### Run

```powershell
# PowerShell — runs both platforms and reports results
.\run_tests.ps1

# Manual — single platform
bin\tests\UnitTests.exe --gtest_output=xml:results.xml
```

The exit code is 0 when all tests pass, non-zero on failure.

## Test Summary

| File | Module Under Test | Tests | Coverage Areas |
|------|-------------------|-------|----------------|
| `ComsubsTests.cpp` | `comsubs.cpp` | 40 | Hex encoding, EBCDIC conversion, string utilities, byte reversal, integer parsing |
| `XmlsubsTests.cpp` | `xmlsubs.cpp` | 29 | XML entity escape/unescape, XML validation, delimiter finding, comment/DTD processing, CR/LF removal |
| `NamesTests.cpp` | `Names.cpp` | 25 | Insert, find, address lookup, iteration, reallocation, statistics |
| `JsonParseTests.cpp` | `JsonParse.cpp` | 19 | JSON parsing, tree navigation, value verification, `buildParsedArea` round-trip, error handling |
| `XMLParseTests.cpp` | `XMLParse.cpp` | 19 | XML parsing, tree navigation, `createXML` round-trip, error messages, structural edge cases |
| **Total** | | **132** | |

## Architecture

### MFC-Hosted Test Runner

The source files under test use MFC types (`CString`, `BOOL`, `AfxGetApp`), so
the test executable must be a Windows application with a live `CWinApp` object.
`TestMain.cpp` defines `CTestApp` (subclass of `CRfhutilApp`) which overrides
`InitInstance()` to run Google Test and then immediately terminates the process.

```
CTestApp::InitInstance()
  → testing::InitGoogleTest()
  → RUN_ALL_TESTS()
  → TerminateProcess()          // skip CRT atexit cleanup
```

`TerminateProcess()` is used instead of a normal return to avoid cross-CRT
shutdown issues between the v143 test exe and the v140 gtest DLLs.

### CRT Compatibility

The gtest NuGet package (`Microsoft.googletest.v140.windesktop.msvcstl.dyn.rt-dyn`)
ships DLLs compiled with the v140 toolset (VS2015) using `/MDd` (dynamic CRT).
The test project uses v143 (VS2022).

**Critical configuration:**
- `UseOfMfc=Dynamic` — uses `/MDd` (dynamic CRT), so the exe shares the same
  `ucrtbased.dll` heap with the gtest DLLs
- Using `UseOfMfc=Static` would default to `/MTd` (static CRT = private heap),
  causing cross-heap corruption and `__acrt_first_block == header` assertion failures

### Stub Headers

The test project uses `prelude.h` (force-included via `/FI`) to stub out MQ SDK
dependencies (`cmqc.h`, `cmqxp.h`, etc.) so tests compile without the IBM MQ
SDK installed.  A local `rfhutil.h` provides minimal type definitions needed by
the source files under test.

### Project Layout

```
tests/UnitTests/
├── UnitTests.vcxproj      # MSBuild project (Debug|Win32, Debug|x64)
├── packages.config         # NuGet: gtest v1.8.1.8
├── TestMain.cpp            # MFC-hosted gtest entry point
├── prelude.h               # MQ SDK stubs (force-included)
├── stdafx.h                # Precompiled header stub
├── rfhutil.h               # Minimal type stubs
├── ComsubsTests.cpp        # Tests for comsubs.cpp
├── XmlsubsTests.cpp        # Tests for xmlsubs.cpp
├── JsonParseTests.cpp      # Tests for JsonParse.cpp
├── XMLParseTests.cpp       # Tests for XMLParse.cpp
└── NamesTests.cpp          # Tests for Names.cpp
```

### Output Locations

| Platform | Executable | XML Results |
|----------|-----------|-------------|
| Win32 | `bin\tests\UnitTests.exe` | `bin\tests\results_win32.xml` |
| x64 | `bin\tests\x64\UnitTests.exe` | `bin\tests\x64\results_x64.xml` |

## Modules Under Test

### comsubs.cpp — Common Utility Subroutines

Pure functions with no MQ or DataArea dependencies.

- **Hex encoding**: `AsciiToHex`, `HexToAscii`, `checkIfHex`, `charValue`, `fromHex`, `getHexCharValue`
- **EBCDIC conversion**: `AsciiToEbcdic`, `EbcdicToAscii`, `isUCS2`
- **String utilities**: `skipWhiteSpace`, `skipBlanks`, `Rtrim`, `findBlank`, `skipQuotedString`, `findcrlf`
- **Integer parsing**: `my_atoi64`
- **Byte reversal**: `reverseBytes`, `reverseBytes4`

### xmlsubs.cpp — XML Helper Subroutines

Standalone XML processing functions.

- **Entity handling**: `removeEscSeq` (unescape), `insertEscChars` (escape), `setEscChar`
- **Validation**: `checkIfXml` (returns 0=valid, non-zero=invalid)
- **Navigation**: `findDelim` (1-indexed, respects escape char), `findEndBracket`, `findXmlValue`
- **Structural**: `processComment` (skip `<!-- -->`), `processDTD` (skip `<!DOCTYPE>`), `removeCrLf`

### JsonParse.cpp — JSON Parser

Tree-based JSON parser with element navigation.

- **Parsing**: `parse()` returns `JPARSE_OK` on success
- **Navigation**: `getFirstChild()`, `getNextSibling()`, `getElemName()`, `getElemValue()`
- **Output**: `buildParsedArea()` reconstructs JSON from the parse tree
- **Note**: `getElemValue()` returns string values WITH surrounding double quotes (e.g., `"\"Alice\""`)
- **Note**: Array elements are stored as the value string, not as navigable children

### XMLParse.cpp — XML Parser

Tree-based XML parser with element navigation.

- **Parsing**: `parse()` returns `PARSE_OK` on success
- **Navigation**: `getFirstChild()`, `getNextSibling()`, `getElemName()`, `getElemValue()`, `getElemType()`
- **Output**: `createXML()` reconstructs XML from the parse tree
- **Error reporting**: `getErrorMsg()` for error codes, `getLastError()` for parse failure details

### Names.cpp — String Name Table

Hash-based name storage with insert, find, and iteration.

- **Insert/Find**: `insertName()` returns handle > 0, `findName()` returns 0 if not found
- **Address lookup**: `getNameAddr()` returns pointer to stored string
- **Iteration**: `getFirstEntry()` / `getNextEntry()` — note: internal extra entry included in count
- **Statistics**: `getInsertCount()`, `getReallocCount()`, `getNameTableSize()`
- **Reallocation**: Small initial table size forces reallocs, all entries remain valid after realloc

## Adding New Tests

1. Create `<Module>Tests.cpp` in `tests/UnitTests/`
2. Add the `.cpp` file to the test files `<ItemGroup>` in `UnitTests.vcxproj`
3. Add the source file under test to the source files `<ItemGroup>` if not already present
4. Add any required headers to the headers `<ItemGroup>`
5. Include `"stdafx.h"`, `"rfhutil.h"`, and `<gtest/gtest.h>` at the top
6. If the source file needs MQ types, add stubs to `prelude.h`

## Known Limitations

- **No MQ SDK tests**: Functions that call MQ APIs (`MQCONN`, `MQPUT`, etc.) cannot
  be tested without the IBM MQ SDK installed and a running queue manager
- **No GUI tests**: Dialog-based UI code (`DataArea`, tab pages) is not tested
- **Parser leniency**: Both JSON and XML parsers are lenient with malformed input —
  some tests document behavior rather than assert specific error codes
- **gtest v1.8.1.8**: Older version (v140 NuGet); does not support parameterized
  test suites or newer gtest features

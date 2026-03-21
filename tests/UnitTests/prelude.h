// prelude.h — force-included before every translation unit in the UnitTests project.
// Purpose: prevent RFHUtil/rfhutil.h from pulling in DataArea.h → cmqc.h (MQ headers).
//
// Problem: source files (comsubs.cpp, JsonParse.cpp, etc.) live in RFHUtil/ and do
//   #include "rfhutil.h"
// MSVC resolves quoted includes relative to the source file's directory first, so it
// always finds RFHUtil/rfhutil.h — not our stub in tests/UnitTests/.
//
// Solution: pre-define rfhutil.h's own include guard so that when the real file is
// found, its entire body (including #include "DataArea.h") is skipped.
// Then provide the minimal CRfhutilApp stub that source files need at compile time.
#pragma once

// Pull in MFC (required by all source files before rfhutil.h).
#include "stdafx.h"

// Names.h is normally pulled in via rfhutil.h → DataArea.h. Since we block that
// chain, include it directly. JsonParse.h and XMLParse.h both embed a Names member.
#include "Names.h"

// Block RFHUtil/rfhutil.h from being processed — the guard is the one defined in
// RFHUtil/rfhutil.h line 22.
#define AFX_RFHUTIL_H__96F2439F_AE02_4650_8A3E_EE7F0EC7E72C__INCLUDED_

// Minimal CRfhutilApp stub.
// comsubs.cpp casts AfxGetApp() to CRfhutilApp* to call isTraceEnabled(),
// logTraceEntry(), verboseTrace, and dumpTraceData(). Return FALSE / no-op so
// trace code is compiled out cleanly.
class CRfhutilApp : public CWinApp
{
public:
    BOOL verboseTrace;
    CRfhutilApp() : verboseTrace(FALSE) {}
    BOOL isTraceEnabled()                                                          { return FALSE; }
    void logTraceEntry(const char *)                                               {}
    void logTraceEntry(const CString&)                                             {}
    void dumpTraceData(const char *, const unsigned char *, unsigned int)          {}
};

// Tab page constants (defined in real rfhutil.h; kept here as a safety net).
#define PAGE_MAIN     0
#define PAGE_DATA     1
#define PAGE_MQMD     2
#define PAGE_CONN    14

// TestMain.cpp — entry point for the MFC-hosted Google Test runner.
//
// Because the unit test source files (comsubs.cpp, xmlsubs.cpp, etc.) use MFC
// types (CString, BOOL, AfxGetApp), the test executable must be a Windows
// application with a live CWinApp object.  We override InitInstance() to
// run Google Test and exit immediately — the MFC message loop is never entered.
//
// CRT mismatch fix  (v140 gtest DLL + v143 exe)
// ------------------------------------------------
// The gtest NuGet package was compiled with v140 (VS2015).  Both the v140
// and v143 VCRUNTIME DLLs share ucrtbased.dll's heap but each registers its
// own CRT atexit/DLL_PROCESS_DETACH cleanup.  The result is two heap-validation
// passes over the same data, causing a spurious
//   "__acrt_first_block == header"
// assertion dialog that fires both at gtestd.dll load-time AND at shutdown.
//
// Two-part fix in this file:
//   1. BEFORE gtestd.dll loads:  set _CrtSetReportMode to redirect the
//      assertion to OutputDebugString (no dialog).  Because gtestd.dll and
//      gtest_maind.dll are DELAY-LOADED (see UnitTests.vcxproj), they don't
//      initialise until the first call into them — which is testing::
//      InitGoogleTest(), called after we set the report mode below.
//   2. AFTER tests finish:  call TerminateProcess() to exit without running
//      CRT atexit / DLL_PROCESS_DETACH, eliminating the shutdown-time copy
//      of the same assertion.
//
// Stdout note
// -----------
// This is a /SUBSYSTEM:WINDOWS exe, so there is no stdout.  Callers must
// pass --gtest_output=xml:<path> so that results are written to a file.

#include "stdafx.h"
#include "rfhutil.h"
#include <gtest/gtest.h>

// Concrete application class.
// AfxGetApp() returns this object, satisfying comsubs.cpp's
// (CRfhutilApp*)AfxGetApp() casts without crashing.
class CTestApp : public CRfhutilApp
{
    int m_testResult = 0;
public:
    BOOL InitInstance() override
    {
#ifdef _DEBUG
        // Part 1: redirect CRT assertion dialogs BEFORE gtestd.dll initialises.
        // Because the gtest DLLs are delay-loaded, this call runs first.
        // _CrtSetReportMode writes into ucrtbased.dll's global state, which is
        // shared by both the v140 (gtest) and v143 (exe) runtimes.
        _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
        _CrtSetReportMode(_CRT_ERROR,  _CRTDBG_MODE_DEBUG);
        _CrtSetReportMode(_CRT_WARN,   _CRTDBG_MODE_DEBUG);
#endif
        // This call triggers the delay-load of gtestd.dll / gtest_maind.dll.
        // The v140 CRT in those DLLs now initialises with our report mode
        // already set, so any assertion fires silently to the debug output
        // stream instead of popping a dialog.
        testing::InitGoogleTest(&__argc, __argv);
        m_testResult = RUN_ALL_TESTS();
        return FALSE;  // skip message loop; go straight to ExitInstance()
    }

    int ExitInstance() override
    {
        // Part 2: exit without CRT cleanup to suppress the shutdown-time
        // "__acrt_first_block == header" assertion.  All test results are
        // already written to the XML file before RUN_ALL_TESTS() returned.
        ::TerminateProcess(::GetCurrentProcess(), static_cast<UINT>(m_testResult));
        return m_testResult;  // unreachable; keeps the compiler happy
    }
};

// The one-and-only application object.  MFC's WinMain (from the static MFC lib)
// picks this up via AfxGetApp() and calls InitInstance().
CTestApp theApp;

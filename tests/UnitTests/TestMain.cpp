// TestMain.cpp — entry point for the MFC-hosted Google Test runner.
//
// Because the unit test source files (comsubs.cpp, xmlsubs.cpp, etc.) use MFC
// types (CString, BOOL, AfxGetApp), the test executable must be a Windows
// application with a live CWinApp object.  We override InitInstance() to
// run Google Test and exit immediately — the MFC message loop is never entered.
//
// Two quirks of the /SUBSYSTEM:WINDOWS + static-MFC + gtest-v140 combination
// are handled here:
//
//  1. No stdout.  A Windows-subsystem exe has no console, so printf/fwrite go
//     nowhere.  gtest's --gtest_output=xml:<path> writes results to a file
//     instead, which is how the test runner captures output.
//
//  2. "__acrt_first_block == header" assertion at shutdown.  The gtest NuGet
//     package was compiled with v140 (VS2015); this exe uses v143 (VS2022).
//     Both runtimes share ucrtbased.dll's heap but each runs its own atexit /
//     DLL_PROCESS_DETACH cleanup.  The second pass finds the heap in a state
//     the first pass already modified, causing a spurious assertion dialog that
//     blocks CI.  Fixed by calling TerminateProcess() in ExitInstance() — all
//     test results are already in the XML file at that point, so no state is
//     lost.

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
        // __argc / __argv are the parsed command-line available in any Win32 app.
        // Callers should pass --gtest_output=xml:<path> so results are written
        // to a file (stdout is not connected in a Windows-subsystem exe).
        testing::InitGoogleTest(&__argc, __argv);
        m_testResult = RUN_ALL_TESTS();
        return FALSE;  // do not enter the message loop; go straight to ExitInstance
    }

    int ExitInstance() override
    {
        // TerminateProcess exits immediately without running CRT atexit handlers
        // or DLL_PROCESS_DETACH callbacks.  This prevents the spurious
        // "__acrt_first_block == header" assertion dialog that fires when the
        // v140 and v143 CRT runtimes both try to validate ucrtbased.dll's heap
        // at shutdown.  All gtest results are already flushed to the XML file
        // before RUN_ALL_TESTS() returned, so nothing is lost.
        ::TerminateProcess(::GetCurrentProcess(), static_cast<UINT>(m_testResult));
        return m_testResult;  // unreachable, satisfies the compiler
    }
};

// The one-and-only application object.  MFC's WinMain (from the static MFC lib)
// picks this up via AfxGetApp() and calls InitInstance().
CTestApp theApp;

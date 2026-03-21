// TestMain.cpp — entry point for the MFC-hosted Google Test runner.
//
// The source files under test (comsubs.cpp, xmlsubs.cpp, etc.) use MFC types
// (CString, BOOL, AfxGetApp), so the test exe must be a Windows application
// with a live CWinApp object.  We override InitInstance() to run Google Test
// and exit immediately — the MFC message loop is never entered.
//
// The gtest NuGet package was compiled with v140 (VS2015) while our test exe
// uses v143 (VS2022).  We link against the RELEASE gtest DLLs (gtest.dll)
// even in Debug builds — release UCRT has no debug-heap validation, so the
// cross-version CRT sharing is harmless.  TerminateProcess() is called after
// tests complete to skip CRT atexit cleanup and avoid any shutdown issues.
//
// This is a /SUBSYSTEM:WINDOWS exe with no stdout — use
// --gtest_output=xml:<path> to capture results.

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
        testing::InitGoogleTest(&__argc, __argv);
        m_testResult = RUN_ALL_TESTS();

        // Kill the process immediately — skip CRT atexit/DLL cleanup to
        // avoid any cross-CRT shutdown issues.  Results are already in XML.
        ::TerminateProcess(::GetCurrentProcess(), static_cast<UINT>(m_testResult));
        return FALSE;  // unreachable; keeps the compiler happy
    }

    int ExitInstance() override
    {
        // Backup in case InitInstance() doesn't reach TerminateProcess.
        ::TerminateProcess(::GetCurrentProcess(), static_cast<UINT>(m_testResult));
        return m_testResult;  // unreachable; keeps the compiler happy
    }
};

// The one-and-only application object.  MFC's WinMain (from the static MFC lib)
// picks this up via AfxGetApp() and calls InitInstance().
CTestApp theApp;

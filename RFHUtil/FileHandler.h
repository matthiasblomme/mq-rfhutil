#pragma once

// P4.4 — passive holder for the current "file under work" in DataArea.
//
// PR A scope: this class only owns the fundamental state — the path
// strings, the message buffer, its size/CCSID/source. File read/write
// settings (m_readfile_ascii, m_read_nocrlf, etc.) move in PR B. The
// file I/O methods (ReadDataFile, WriteDataFile, ReadFileData,
// WriteFile, changeUnixFile, clearFileData) move in PR C.
//
// During PRs A-C, DataArea exposes the same field names (`fileName`,
// `fileData`, `fileSize`, `fileCcsid`, `fileSource`, `lastFileRead`)
// as references that point into this struct, so existing code in
// DataArea.cpp / General.cpp / MSGDATA.cpp / PS.cpp / PubSub.cpp /
// rfhutilView.cpp keeps compiling unchanged.
//
// PR D (final) migrates the ~44 external call sites to use
// `pDoc->m_file.X` directly and retires the reference aliases.
class FileHandler
{
public:
    FileHandler();

    char           fileName[512];      // current file path
    char           lastFileRead[512];  // last successful read, for refresh
    UINT           fileSize;
    int            fileCcsid;
    unsigned char* fileData;
    CString        fileSource;
};

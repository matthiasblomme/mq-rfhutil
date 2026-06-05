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

    // PR B: file-format read/write settings — knobs that control how the
    // buffer above is interpreted on disk.
    struct Settings {
        // Read options
        BOOL readfile_ascii;       // treat file as ASCII (no binary conversion)
        BOOL read_nocrlf;          // skip CRLF normalization
        BOOL read_unix;            // file written in Unix newline format
        BOOL read_ignore_header;   // skip any MQ header in the file on read
        // Write options
        BOOL dataonly;             // write just the user data, no headers
        BOOL save_rfh;             // include RFH header in the saved file
        // Encoding
        int  codepage;             // CCSID of the file's data
    } settings;
};

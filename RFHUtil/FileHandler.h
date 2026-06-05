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

    // ─── PR C: clean file-buffer / file-system primitives ─────────────────
    // These don't touch DataArea's trace surface, error formatting, or
    // header-parsing state. The DataArea wrappers (changeUnixFile,
    // openOutputFile) keep their thin orchestration role; the bigger
    // orchestrators (ReadFileData, WriteFile, ReadDataFile, WriteDataFile)
    // stay on DataArea because they're entangled with MQ-header building
    // and MFC file-dialog UI — same call we made on connect2QM in P4.1.

    // Was DataArea::changeUnixFile — converts the in-memory buffer from
    // mixed/LF newlines to CRLF in place (with reallocation). Uses
    // rfhMalloc / rfhFree for buffer management.
    void changeUnixFile();

    // Was DataArea::openOutputFile — opens fname for binary write, fills
    // errMsg (if non-NULL) with a descriptive message on failure. Static
    // because the operation doesn't touch FileHandler instance state.
    // Trace logging stays in the DataArea wrapper.
    static FILE* openOutputFile(LPCTSTR fname, LPTSTR errMsg);
};

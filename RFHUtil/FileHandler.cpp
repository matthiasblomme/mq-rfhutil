#include "stdafx.h"
#include "FileHandler.h"
#include "comsubs.h"  // rfhMalloc / rfhFree (PR C)

// P4.4 PR A: passive holder. Defaults match the zero-initialized state
// DataArea relied on before the move.
FileHandler::FileHandler()
    : fileSize(0)
    , fileCcsid(0)
    , fileData(NULL)
{
    fileName[0]     = 0;
    lastFileRead[0] = 0;

    // PR B: file-format settings — defaults match what DataArea's ctor used
    // to set (all FALSE / 0 = "treat as binary, no special handling").
    settings.readfile_ascii     = FALSE;
    settings.read_nocrlf        = FALSE;
    settings.read_unix          = FALSE;
    settings.read_ignore_header = FALSE;
    settings.dataonly           = FALSE;
    settings.save_rfh           = FALSE;
    settings.codepage           = 0;
}

// ─────────────────────────────────────────────────────────────────────────
// PR C: file-buffer / file-system primitives — moved from DataArea.cpp.
// Bodies are byte-identical to the pre-move versions; the only thing
// stripped is trace logging (the DataArea wrappers handle that).
// ─────────────────────────────────────────────────────────────────────────

void FileHandler::changeUnixFile()
{
    if (fileSize == 0)
        return;

    unsigned char* newArea = (unsigned char*)rfhMalloc(fileSize * 2 + 2, "UNIXDATA");

    unsigned int i = 0;
    unsigned int j = 0;
    while (i < fileSize)
    {
        // Existing \r\n pair — copy both bytes.
        if (fileData[i] == '\r' && fileData[i + 1] == '\n')
        {
            newArea[j++] = fileData[i++];
            newArea[j++] = fileData[i++];
        }
        else
        {
            // Bare \n — insert a preceding \r.
            if (fileData[i] == '\n')
                newArea[j++] = '\r';
            newArea[j++] = fileData[i++];
        }
    }

    newArea[j]     = 0;
    newArea[j + 1] = 0;

    rfhFree(fileData);
    fileData = newArea;
    fileSize = j;
}

FILE* FileHandler::openOutputFile(LPCTSTR fname, LPTSTR errMsg)
{
    if (errMsg != NULL)
        errMsg[0] = 0;

    FILE* outputFile = fopen(fname, "wb");

    if (outputFile == NULL && errMsg != NULL)
    {
        DWORD errCode = GetLastError();
        switch (errCode)
        {
        case 32: strcpy(errMsg, "Unable to open output file - sharing violation"); break;
        case 33: strcpy(errMsg, "Unable to open output file - file locked");        break;
        case 39: strcpy(errMsg, "Unable to open output file - disk full");          break;
        case 3:  strcpy(errMsg, "Unable to open output file - path not found");     break;
        case 5:  strcpy(errMsg, "Unable to open output file - access denied");      break;
        default: sprintf(errMsg, "Unable to open output file - Error %d", errCode); break;
        }
    }

    return outputFile;
}

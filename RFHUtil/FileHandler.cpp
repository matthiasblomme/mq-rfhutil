#include "stdafx.h"
#include "FileHandler.h"

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

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
}

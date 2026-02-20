# SAFE_MODE Implementation for rfhutilc

## Overview
This document describes the implementation of SAFE_MODE (browse-only) build configuration for rfhutilc. This mode restricts the application to read-only operations, preventing any destructive or write operations to MQ queues.

## Build Configuration

### New Configuration: ReleaseSafe
- **Output**: `bin\ReleaseSafe\rfhutilc-safe.exe`
- **Preprocessor Definition**: `SAFE_MODE`
- **Based on**: Release configuration with additional restrictions

### Build Commands

Build the safe mode version:
```bash
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" RFHUtil.sln /t:Client:Rebuild /p:Configuration=ReleaseSafe /p:Platform=Win32 /v:minimal
```

## Disabled Operations in SAFE_MODE

### Write Operations (Completely Disabled)
1. **Write Q** (`IDC_WRITEQ`, `ID_WRITEQ`) - Writing messages to queues
2. **Load Q** (`IDC_MAIN_LOADQ`, `ID_LOADQ`) - Loading messages from files to queues
3. **Move Q** (`IDC_MAIN_MOVEQ`) - Moving messages between queues
4. **Clear/Purge Q** (`ID_PURGEQ`, `IDC_PURGE`) - Clearing all messages from a queue
5. **Clear All** (`IDC_CLEAR_ALL`, `ID_CLEARALL`) - Clearing all data
6. **Publish** (`ID_PS_WRITE_MSGS`, `IDC_PUBLISH`) - Publishing messages
7. **Write Pubs** - Writing publications

### Modified Operations (Browse Mode Only)
1. **Read Q** (`IDC_READQ`, `ID_READQ`) - Modified to use MQGMO_BROWSE_FIRST/NEXT instead of destructive get
2. **Start Browse** (`IDC_STARTBR`, `ID_STARTBR`) - Remains enabled (already non-destructive)
3. **Browse Next** (`IDC_BRNEXT`) - Remains enabled (already non-destructive)
4. **Browse Previous** (`IDC_BRPREV`) - Remains enabled (already non-destructive)

### Enabled Operations (Read-Only)
1. **Save Q** (`IDC_MAIN_SAVEQ`, `ID_SAVEQ`) - Saving messages to files (read-only operation)
2. **Display Q** (`IDC_MAIN_DISPLAYQ`) - Displaying queue information
3. **Browse operations** - All browse operations remain functional
4. **Read File** (`ID_READ_FILE`) - Reading data from files
5. **Write File** (`IDC_WRITE_FILE`) - Writing data to files (not queue operations)
6. **Close Q** (`IDC_CLOSEQ`, `ID_CLOSEQ`) - Closing queue connections

## Implementation Details

### UI Controls to Disable
The following buttons/controls will be disabled in SAFE_MODE:

```cpp
#ifdef SAFE_MODE
// Disable write operation buttons
GetDlgItem(IDC_WRITEQ)->EnableWindow(FALSE);
GetDlgItem(IDC_MAIN_LOADQ)->EnableWindow(FALSE);
GetDlgItem(IDC_MAIN_MOVEQ)->EnableWindow(FALSE);
GetDlgItem(IDC_PURGE)->EnableWindow(FALSE);
GetDlgItem(IDC_CLEAR_ALL)->EnableWindow(FALSE);
GetDlgItem(IDC_PUBLISH)->EnableWindow(FALSE);
#endif
```

### Read Q Modification
In SAFE_MODE, the Read Q operation will be modified to use browse mode:

```cpp
void General::OnReadq()
{
#ifdef SAFE_MODE
    // In safe mode, use browse instead of destructive get
    pDoc->browseQ(Q_OPEN_READ_BROWSE);
#else
    // Normal mode - destructive get
    pDoc->readQ(Q_OPEN_READ);
#endif
}
```

### MQGET Options
In SAFE_MODE, all MQGET operations will use:
- `MQGMO_BROWSE_FIRST` for first message
- `MQGMO_BROWSE_NEXT` for subsequent messages
- Never use `MQGMO_MSG_UNDER_CURSOR` with destructive get

## Files to Modify

### 1. General.cpp
- Modify `OnInitDialog()` to disable write buttons in SAFE_MODE
- Modify `OnReadq()` to use browse mode in SAFE_MODE
- Add conditional compilation for write operations

### 2. rfhutilView.cpp
- Modify menu handlers to disable write operations
- Update `OnWriteq()`, `OnLoadq()`, `OnClearall()` to check SAFE_MODE

### 3. DataArea.cpp
- Modify `readQ()` to use browse mode in SAFE_MODE
- Ensure all MQGET operations respect SAFE_MODE

### 4. MainFrm.cpp (if applicable)
- Disable menu items for write operations in SAFE_MODE

## Testing Checklist

- [ ] Build succeeds with ReleaseSafe configuration
- [ ] rfhutilc-safe.exe is created in bin\ReleaseSafe\
- [ ] Write Q button is disabled
- [ ] Load Q button is disabled
- [ ] Move Q button is disabled
- [ ] Purge Q button is disabled
- [ ] Clear All button is disabled
- [ ] Read Q uses browse mode (non-destructive)
- [ ] Browse operations work normally
- [ ] Save Q works (saves messages to files)
- [ ] Display Q works
- [ ] No MQPUT operations are possible
- [ ] No destructive MQGET operations are possible

## Security Benefits

1. **Audit Trail**: Safe mode can be deployed in production for read-only access
2. **Training**: New users can explore queues without risk of data loss
3. **Troubleshooting**: Support teams can investigate issues without modifying data
4. **Compliance**: Meets requirements for read-only access in regulated environments

## Version Information

- **Initial Implementation**: v9.4.0.0
- **Configuration**: ReleaseSafe|Win32
- **Output Binary**: rfhutilc-safe.exe
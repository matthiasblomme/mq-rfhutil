# IDPWOS Authentication Fix - Implementation Plan

## Overview

This document provides a detailed implementation plan to fix the IDPWOS authentication issue where the new RFHUtil fails to authenticate against LDAP-backed queue managers while the old version works.

**Root Cause:** Changed from `GetUserNameEx(NameSamCompatible, ...)` to `GetUserName()`, removing domain prefix from username.

**Related Documentation:** See [`IDPWOS_AUTHENTICATION_ISSUE.md`](IDPWOS_AUTHENTICATION_ISSUE.md) for detailed analysis.

## Implementation Strategy

### Recommended Approach: Restore GetUserNameEx with Buffer Enhancement

This approach restores the original working behavior while addressing the 12-character truncation issue.

## Code Changes Required

### Change 1: Fix Username Retrieval in MQMDPAGE.cpp

**File:** [`RFHUtil/MQMDPAGE.cpp`](../RFHUtil/MQMDPAGE.cpp)  
**Location:** Lines 2570-2581  
**Function:** `MQMDPAGE::buildMQMD()`

#### Current Code (Broken)
```cpp
else
{
    // we need to set the first 12 characters of the logged in user id
    memset(userId, 0, sizeof(userId));
//  rc = GetUserNameEx(NameSamCompatible, userId, &userIdSize);
    rc = GetUserName(userId, &userIdSize);
    if (rc != 0)
    {
        userId[12] = 0;
        setUserId(mqmd);
    }
}
```

#### Proposed Fix
```cpp
else
{
    // Get the domain-qualified user id for IDPWOS authentication
    // Use a larger buffer to accommodate DOMAIN\username format
    char fullUserId[256];
    DWORD fullUserIdSize = sizeof(fullUserId);
    memset(fullUserId, 0, sizeof(fullUserId));
    memset(userId, 0, sizeof(userId));
    
    // Try to get domain-qualified username first (for LDAP/IDPWOS)
    rc = GetUserNameEx(NameSamCompatible, fullUserId, &fullUserIdSize);
    if (rc != 0)
    {
        // Successfully got domain\username format
        // Copy to userId buffer, truncating if necessary to MQ_USER_ID_LENGTH (12)
        strncpy(userId, fullUserId, MQ_USER_ID_LENGTH);
        userId[MQ_USER_ID_LENGTH] = 0;
        setUserId(mqmd);
    }
    else
    {
        // Fallback to simple username if GetUserNameEx fails
        // This handles cases where domain info is not available
        DWORD simpleUserIdSize = sizeof(userId);
        rc = GetUserName(userId, &simpleUserIdSize);
        if (rc != 0)
        {
            userId[MQ_USER_ID_LENGTH] = 0;
            setUserId(mqmd);
        }
    }
}
```

#### Implementation Notes

1. **Buffer Size:** Increased to 256 bytes to accommodate full domain\username
2. **Fallback Logic:** If `GetUserNameEx` fails, fall back to `GetUserName` for compatibility
3. **Truncation:** Uses `MQ_USER_ID_LENGTH` constant instead of hardcoded 12
4. **Safety:** Uses `strncpy` to prevent buffer overflow

### Change 2: Add Required Header

**File:** [`RFHUtil/MQMDPAGE.cpp`](../RFHUtil/MQMDPAGE.cpp)  
**Location:** Top of file with other includes

#### Add Include
```cpp
#include <Secext.h>  // For GetUserNameEx
```

#### Link Library
Ensure the project links against `Secur32.lib`:
- **RFHUtil.vcxproj:** Add to `<AdditionalDependencies>`
- **Client.vcxproj:** Add to `<AdditionalDependencies>`

```xml
<AdditionalDependencies>Secur32.lib;version.lib;%(AdditionalDependencies)</AdditionalDependencies>
```

## Alternative Implementation Options

### Option A: Environment Variable Control (More Flexible)

Add configuration option to choose username format:

```cpp
else
{
    char fullUserId[256];
    DWORD fullUserIdSize = sizeof(fullUserId);
    memset(fullUserId, 0, sizeof(fullUserId));
    memset(userId, 0, sizeof(userId));
    
    // Check if user wants domain-qualified username (default: yes)
    char* useDomain = getenv("RFHUTIL_USE_DOMAIN_USERNAME");
    BOOL shouldUseDomain = (useDomain == NULL) || (strcmp(useDomain, "0") != 0);
    
    if (shouldUseDomain)
    {
        rc = GetUserNameEx(NameSamCompatible, fullUserId, &fullUserIdSize);
        if (rc != 0)
        {
            strncpy(userId, fullUserId, MQ_USER_ID_LENGTH);
            userId[MQ_USER_ID_LENGTH] = 0;
            setUserId(mqmd);
        }
        else
        {
            // Fallback to simple username
            DWORD simpleUserIdSize = sizeof(userId);
            rc = GetUserName(userId, &simpleUserIdSize);
            if (rc != 0)
            {
                userId[MQ_USER_ID_LENGTH] = 0;
                setUserId(mqmd);
            }
        }
    }
    else
    {
        // Use simple username (old broken behavior)
        DWORD simpleUserIdSize = sizeof(userId);
        rc = GetUserName(userId, &simpleUserIdSize);
        if (rc != 0)
        {
            userId[MQ_USER_ID_LENGTH] = 0;
            setUserId(mqmd);
        }
    }
}
```

**Environment Variable:**
- `RFHUTIL_USE_DOMAIN_USERNAME=1` (default) - Use domain\username
- `RFHUTIL_USE_DOMAIN_USERNAME=0` - Use simple username

### Option B: Registry Setting (Windows-Native)

Similar to Option A but using Windows Registry instead of environment variable:

```cpp
// Read from registry: HKEY_CURRENT_USER\Software\IBM\RFHUtil\UseDomainUsername
HKEY hKey;
DWORD useDomain = 1;  // Default to domain-qualified
DWORD dataSize = sizeof(DWORD);

if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\IBM\\RFHUtil", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
{
    RegQueryValueEx(hKey, "UseDomainUsername", NULL, NULL, (LPBYTE)&useDomain, &dataSize);
    RegCloseKey(hKey);
}
```

## Testing Plan

### Unit Testing

#### Test Case 1: Domain-Qualified Username
**Setup:**
- Windows domain environment
- User logged in as `CONTOSO\bmatt`

**Expected:**
- `GetUserNameEx` succeeds
- Username set to `CONTOSO\bmat` (truncated to 12 chars)
- Authentication succeeds with LDAP

#### Test Case 2: Local Account
**Setup:**
- Local Windows account (no domain)
- User logged in as `bmatt`

**Expected:**
- `GetUserNameEx` fails gracefully
- Falls back to `GetUserName`
- Username set to `bmatt`
- Authentication behavior depends on LDAP config

#### Test Case 3: Long Username
**Setup:**
- Domain: `VERYLONGDOMAIN`
- Username: `verylongusername`
- Full: `VERYLONGDOMAIN\verylongusername` (33 chars)

**Expected:**
- Username truncated to `VERYLONGDOMA` (12 chars)
- Warning logged about truncation
- Authentication may fail if LDAP requires full name

### Integration Testing

#### Test Environment Setup
```
Queue Manager: QM1
CHLAUTH: Enabled
CONNAUTH: SYSTEM.DEFAULT.AUTHINFO.IDPWOS
Auth Info: LDAP backend
Test Queue: TEST.QUEUE
```

#### Test Scenarios

| Scenario | User Type | Expected Result |
|----------|-----------|-----------------|
| TS1 | Domain user, LDAP configured | Success |
| TS2 | Local user, LDAP configured | Depends on LDAP config |
| TS3 | Domain user, no LDAP | Success (local auth) |
| TS4 | Username > 12 chars | Truncation warning, may fail |

### Regression Testing

Verify existing functionality still works:
- [ ] Connection with explicit userid/password (MQCSP)
- [ ] Connection without userid (default context)
- [ ] Alternate user authority (MQOO_ALTERNATE_USER_AUTHORITY)
- [ ] Set User ID checkbox functionality
- [ ] Set All Context checkbox functionality

## Deployment Considerations

### Backward Compatibility

**Impact:** This change restores the old behavior, so it should improve compatibility with existing LDAP configurations.

**Risk Areas:**
1. Environments that adapted to the broken behavior
2. Truncation of long domain\username combinations
3. Systems without domain (will fall back to simple username)

### Rollout Strategy

1. **Phase 1:** Deploy to test environment
   - Verify with LDAP-backed queue managers
   - Test both domain and local accounts
   - Monitor authentication logs

2. **Phase 2:** Limited production deployment
   - Deploy to subset of users
   - Gather feedback on authentication success
   - Monitor for any new issues

3. **Phase 3:** Full deployment
   - Roll out to all users
   - Update documentation
   - Provide troubleshooting guide

### Documentation Updates

Update the following documentation:
- [ ] User guide - explain IDPWOS authentication behavior
- [ ] Troubleshooting guide - add section on authentication issues
- [ ] Release notes - document the fix
- [ ] README - mention LDAP/IDPWOS support

## Troubleshooting Guide

### Issue: Authentication Still Fails After Fix

**Possible Causes:**
1. Username truncated due to long domain name
2. LDAP expects different username format (e.g., UPN: user@domain.com)
3. CHLAUTH rules blocking the connection
4. LDAP server configuration issue

**Diagnostic Steps:**
1. Enable MQ trace: `strmqtrc -m QM1 -t all`
2. Check username in trace: Look for MQOPEN/MQPUT calls
3. Check LDAP server logs for authentication attempts
4. Verify CHLAUTH rules: `DISPLAY CHLAUTH(*)`
5. Test with explicit userid in connection dialog

**Workarounds:**
1. Use explicit userid/password in connection settings
2. Set environment variable `RFHUTIL_USE_DOMAIN_USERNAME=0` (if Option A implemented)
3. Modify LDAP to accept simple usernames
4. Adjust CHLAUTH rules to allow the user

### Issue: GetUserNameEx Fails

**Possible Causes:**
1. Not in a domain environment
2. Secur32.lib not linked
3. Windows version too old (pre-XP)

**Solution:**
- Code automatically falls back to `GetUserName()`
- Verify fallback logic is working
- Check application event log for errors

## Code Review Checklist

- [ ] `GetUserNameEx` properly declared with `#include <Secext.h>`
- [ ] `Secur32.lib` added to linker dependencies
- [ ] Buffer sizes adequate (256 bytes for full username)
- [ ] Proper null termination of strings
- [ ] Fallback logic handles `GetUserNameEx` failure
- [ ] Truncation uses `MQ_USER_ID_LENGTH` constant
- [ ] No buffer overflows possible
- [ ] Error handling for all API calls
- [ ] Trace logging added for debugging
- [ ] Comments explain the domain-qualified username requirement

## Success Criteria

- [ ] Authentication succeeds with LDAP-backed IDPWOS
- [ ] Domain-qualified username sent to MQ
- [ ] Fallback works for non-domain environments
- [ ] No regression in existing authentication methods
- [ ] All unit tests pass
- [ ] All integration tests pass
- [ ] Documentation updated

## Timeline Estimate

- **Code Changes:** 2-4 hours
- **Unit Testing:** 4-6 hours
- **Integration Testing:** 8-12 hours
- **Documentation:** 2-4 hours
- **Code Review:** 2-3 hours
- **Total:** 18-29 hours (2.5-4 days)

## References

- [IDPWOS Authentication Issue Analysis](IDPWOS_AUTHENTICATION_ISSUE.md)
- [GetUserNameEx Documentation](https://docs.microsoft.com/en-us/windows/win32/api/secext/nf-secext-getusernameexa)
- [MQ Security Documentation](https://www.ibm.com/docs/en/ibm-mq/latest?topic=security-channel-authentication-records)
- [LDAP Authentication in MQ](https://www.ibm.com/docs/en/ibm-mq/latest?topic=mechanisms-ldap-authentication)
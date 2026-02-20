# IDPWOS Authentication Fix - Testing Guide

## Overview

This guide provides comprehensive testing procedures to verify the IDPWOS authentication fix works correctly across different environments and scenarios.

**Related Documents:**
- [Issue Analysis](IDPWOS_AUTHENTICATION_ISSUE.md)
- [Implementation Plan](IDPWOS_FIX_IMPLEMENTATION_PLAN.md)

## Test Environment Requirements

### Minimum Test Environment

#### Queue Manager Configuration
```
Queue Manager: QM_TEST
MQ Version: 8.0 or later
CHLAUTH: Enabled
CONNAUTH: SYSTEM.DEFAULT.AUTHINFO.IDPWOS
```

#### LDAP Configuration
```
Auth Info Object: SYSTEM.DEFAULT.AUTHINFO.IDPWOS
LDAP Server: Active Directory or OpenLDAP
Base DN: DC=contoso,DC=com
User Format: DOMAIN\username or username@domain.com
```

#### Test Queues
```
TEST.AUTH.QUEUE - For basic authentication tests
TEST.AUTH.SECURE - For CHLAUTH rule tests
TEST.AUTH.ADMIN - For admin authority tests
```

### Test User Accounts

| Account Type | Username | Domain | Purpose |
|-------------|----------|--------|---------|
| Domain User | testuser1 | CONTOSO | Standard domain authentication |
| Domain User | testuser2 | CONTOSO | Alternative domain user |
| Local User | localuser | (none) | Non-domain authentication |
| Long Name | verylongusername | VERYLONGDOMAIN | Truncation testing |
| Admin User | mqadmin | CONTOSO | Administrative operations |

## Pre-Test Verification

### 1. Verify Old Version Behavior

**Purpose:** Establish baseline of working authentication

**Steps:**
1. Use old RFHUtil C version (rfhutilc.exe from backup)
2. Connect to queue manager with IDPWOS enabled
3. Enable "Set User ID" checkbox on General tab
4. Put a message to TEST.AUTH.QUEUE
5. Verify message is accepted

**Expected Result:**
- Connection succeeds
- Message put succeeds
- MQMD UserIdentifier shows domain\username

**Capture:**
```
MQ Trace: strmqtrc -m QM_TEST -t all
LDAP Logs: Check authentication attempts
RFHUtil Trace: Enable via rfhTrace.cmd
```

### 2. Verify Broken Behavior

**Purpose:** Confirm the issue exists in current version

**Steps:**
1. Use current RFHUtil version (before fix)
2. Repeat steps from Pre-Test 1

**Expected Result:**
- Connection may succeed
- Message put fails with authentication error
- MQMD UserIdentifier shows username without domain

**Error Codes to Watch:**
- MQRC_NOT_AUTHORIZED (2035)
- MQRC_SECURITY_ERROR (2063)

## Test Cases

### TC1: Basic Domain Authentication

**Objective:** Verify domain-qualified username is sent correctly

**Prerequisites:**
- Domain user account (CONTOSO\testuser1)
- LDAP configured and accessible
- CHLAUTH rules allow the user

**Test Steps:**
1. Launch fixed RFHUtil version
2. Connect to QM_TEST
3. Navigate to General tab
4. Check "Set User ID" checkbox
5. Navigate to MQMD tab
6. Leave User ID field empty (to use Windows username)
7. Navigate to Data tab
8. Enter test message: "Test message 1"
9. Click "Write Q" button
10. Enable MQ trace before step 9
11. Check trace for username format

**Expected Results:**
- ✓ Connection succeeds
- ✓ Message written successfully
- ✓ MQMD UserIdentifier contains "CONTOSO\test" (truncated to 12 chars)
- ✓ LDAP logs show successful authentication
- ✓ No MQRC_NOT_AUTHORIZED errors

**Verification:**
```bash
# Check message on queue
echo "DISPLAY QSTATUS(TEST.AUTH.QUEUE) CURDEPTH" | runmqsc QM_TEST

# Browse message and check MQMD
# Use RFHUtil to browse and verify UserIdentifier field
```

### TC2: Local Account Fallback

**Objective:** Verify fallback to simple username for non-domain accounts

**Prerequisites:**
- Local user account (localuser)
- Logged in as local user
- LDAP may or may not accept simple usernames

**Test Steps:**
1. Log in to Windows as localuser
2. Launch fixed RFHUtil
3. Connect to QM_TEST
4. Enable "Set User ID"
5. Attempt to write message

**Expected Results:**
- ✓ GetUserNameEx fails (no domain)
- ✓ Falls back to GetUserName
- ✓ Username set to "localuser"
- ✓ Authentication result depends on LDAP configuration
  - If LDAP accepts simple names: Success
  - If LDAP requires domain: Fails with MQRC_NOT_AUTHORIZED

**Note:** This is expected behavior - local accounts may not work with LDAP

### TC3: Long Username Truncation

**Objective:** Verify proper handling of usernames exceeding 12 characters

**Prerequisites:**
- Domain user with long name (VERYLONGDOMAIN\verylongusername)
- Total length > 12 characters

**Test Steps:**
1. Log in as long username user
2. Launch fixed RFHUtil
3. Enable MQ trace
4. Connect and write message with "Set User ID" enabled
5. Check trace for actual username sent

**Expected Results:**
- ✓ Username truncated to 12 characters
- ✓ Truncation is "VERYLONGDOM" (first 12 chars of domain\username)
- ✓ Warning logged about truncation (if implemented)
- ✓ Authentication may fail if LDAP requires full username

**Verification:**
```
Check MQ trace for MQOPEN/MQPUT calls
Look for UserIdentifier field in MQOD/MQMD structures
```

### TC4: Explicit User ID Override

**Objective:** Verify explicit user ID in MQMD field takes precedence

**Prerequisites:**
- Domain user logged in
- Different user ID to specify

**Test Steps:**
1. Connect to QM_TEST
2. Enable "Set User ID"
3. Navigate to MQMD tab
4. Enter explicit User ID: "TESTUSER2"
5. Write message

**Expected Results:**
- ✓ Explicit user ID used instead of Windows username
- ✓ MQMD UserIdentifier shows "TESTUSER2"
- ✓ Authentication uses specified user ID

### TC5: Alternate User Authority

**Objective:** Verify alternate user authority with domain username

**Prerequisites:**
- User has authority to use alternate user ID
- MQOO_ALTERNATE_USER_AUTHORITY permission granted

**Test Steps:**
1. Connect to QM_TEST
2. Navigate to General tab
3. Check "Alternate User ID" checkbox
4. Navigate to MQMD tab
5. Enter alternate user: "CONTOSO\mqadmin"
6. Write message

**Expected Results:**
- ✓ Queue opened with MQOO_ALTERNATE_USER_AUTHORITY
- ✓ Alternate user ID used for authorization
- ✓ Domain-qualified username accepted

### TC6: Connection User ID (MQCSP)

**Objective:** Verify connection authentication still works independently

**Prerequisites:**
- Valid connection credentials

**Test Steps:**
1. Launch RFHUtil
2. Before connecting, click "Set Conn User" button
3. Enter userid: "CONTOSO\testuser1"
4. Enter password: (valid password)
5. Connect to QM_TEST
6. Write message WITHOUT "Set User ID" enabled

**Expected Results:**
- ✓ Connection succeeds using MQCSP
- ✓ Message written with default context
- ✓ MQMD UserIdentifier shows connection user or blank
- ✓ This path is independent of the fix

### TC7: Multiple Message Operations

**Objective:** Verify username handling across multiple operations

**Test Steps:**
1. Connect with "Set User ID" enabled
2. Write 10 messages sequentially
3. Browse messages
4. Read messages
5. Verify each operation uses correct username

**Expected Results:**
- ✓ All operations succeed
- ✓ Consistent username across operations
- ✓ No authentication errors

### TC8: Regression - Without Set User ID

**Objective:** Verify normal operation without Set User ID

**Test Steps:**
1. Connect to QM_TEST
2. Do NOT enable "Set User ID"
3. Write messages normally

**Expected Results:**
- ✓ Messages written successfully
- ✓ Default context used
- ✓ No impact from the fix

### TC9: Different Username Formats in LDAP

**Objective:** Test various LDAP username format expectations

**Test Configurations:**

| LDAP Format | Test Username | Expected Result |
|-------------|---------------|-----------------|
| DOMAIN\user | CONTOSO\testuser1 | Success |
| user@domain | testuser1@contoso.com | May fail (format mismatch) |
| CN=user,OU=... | CN=testuser1,OU=Users,DC=contoso,DC=com | May fail (format mismatch) |
| Simple username | testuser1 | Depends on LDAP config |

**Note:** The fix provides DOMAIN\username format. If LDAP expects different format, additional configuration may be needed.

### TC10: Cross-Platform Testing

**Objective:** Verify fix works on different Windows versions

**Test Platforms:**
- Windows 10 (latest)
- Windows 11
- Windows Server 2019
- Windows Server 2022

**For Each Platform:**
1. Run TC1 (Basic Domain Authentication)
2. Verify GetUserNameEx availability
3. Check for any platform-specific issues

## Performance Testing

### PT1: Authentication Performance

**Objective:** Verify no performance degradation

**Test Steps:**
1. Write 1000 messages with "Set User ID" enabled
2. Measure time taken
3. Compare with old version
4. Compare with version without "Set User ID"

**Expected Results:**
- ✓ Performance similar to old version
- ✓ GetUserNameEx adds negligible overhead
- ✓ No memory leaks

### PT2: Concurrent Users

**Objective:** Verify multiple users can authenticate simultaneously

**Test Steps:**
1. Launch 10 RFHUtil instances
2. Each with different domain user
3. All enable "Set User ID"
4. All write messages concurrently

**Expected Results:**
- ✓ All instances authenticate successfully
- ✓ No username conflicts
- ✓ Each message has correct user ID

## Security Testing

### ST1: Username Injection

**Objective:** Verify no username injection vulnerabilities

**Test Steps:**
1. Attempt to set username with special characters
2. Try: "DOMAIN\user;DROP TABLE"
3. Try: "DOMAIN\user\x00admin"
4. Verify proper sanitization

**Expected Results:**
- ✓ Special characters handled safely
- ✓ No buffer overflows
- ✓ No SQL injection (if LDAP uses SQL backend)

### ST2: Password Handling

**Objective:** Verify passwords not logged or exposed

**Test Steps:**
1. Enable all tracing
2. Connect with explicit password
3. Review all log files
4. Verify password not in clear text

**Expected Results:**
- ✓ Password not in MQ trace
- ✓ Password not in RFHUtil trace
- ✓ Password not in LDAP logs (should be hashed)

## Error Handling Testing

### ET1: LDAP Server Unavailable

**Test Steps:**
1. Stop LDAP server
2. Attempt to connect with "Set User ID"
3. Verify error handling

**Expected Results:**
- ✓ Graceful error message
- ✓ No application crash
- ✓ Clear indication of LDAP issue

### ET2: Invalid Credentials

**Test Steps:**
1. Use valid username but wrong password
2. Attempt operations

**Expected Results:**
- ✓ MQRC_NOT_AUTHORIZED error
- ✓ Clear error message
- ✓ No security information leaked

### ET3: GetUserNameEx Failure

**Test Steps:**
1. Simulate GetUserNameEx failure (if possible)
2. Verify fallback to GetUserName

**Expected Results:**
- ✓ Automatic fallback
- ✓ Operation continues with simple username
- ✓ Warning logged

## Test Execution Checklist

### Pre-Execution
- [ ] Test environment configured
- [ ] Test users created
- [ ] LDAP server accessible
- [ ] Queue manager running
- [ ] Test queues created
- [ ] Old version backed up
- [ ] Tracing enabled

### Execution
- [ ] TC1: Basic Domain Authentication
- [ ] TC2: Local Account Fallback
- [ ] TC3: Long Username Truncation
- [ ] TC4: Explicit User ID Override
- [ ] TC5: Alternate User Authority
- [ ] TC6: Connection User ID (MQCSP)
- [ ] TC7: Multiple Message Operations
- [ ] TC8: Regression - Without Set User ID
- [ ] TC9: Different Username Formats
- [ ] TC10: Cross-Platform Testing
- [ ] PT1: Authentication Performance
- [ ] PT2: Concurrent Users
- [ ] ST1: Username Injection
- [ ] ST2: Password Handling
- [ ] ET1: LDAP Server Unavailable
- [ ] ET2: Invalid Credentials
- [ ] ET3: GetUserNameEx Failure

### Post-Execution
- [ ] All test results documented
- [ ] Issues logged
- [ ] Performance metrics recorded
- [ ] Security review completed
- [ ] Test environment cleaned up

## Test Result Template

```
Test Case: TC1 - Basic Domain Authentication
Date: YYYY-MM-DD
Tester: [Name]
Environment: [QM name, LDAP server]
RFHUtil Version: [Version number]

Steps Executed:
1. [Step 1] - PASS/FAIL
2. [Step 2] - PASS/FAIL
...

Results:
- Expected: [Description]
- Actual: [Description]
- Status: PASS/FAIL

Evidence:
- Screenshot: [filename]
- Trace file: [filename]
- LDAP log: [filename]

Issues Found:
- [Issue 1]
- [Issue 2]

Notes:
[Any additional observations]
```

## Troubleshooting During Testing

### Issue: Authentication Still Fails

**Check:**
1. Verify fix was applied correctly
2. Check GetUserNameEx is being called (add debug logging)
3. Verify Secur32.lib is linked
4. Check LDAP server logs for username format received
5. Verify CHLAUTH rules

### Issue: Username Truncated Incorrectly

**Check:**
1. Verify buffer size (should be 256 bytes)
2. Check truncation logic uses MQ_USER_ID_LENGTH
3. Verify strncpy used correctly
4. Check for off-by-one errors

### Issue: Fallback Not Working

**Check:**
1. Verify GetUserName called when GetUserNameEx fails
2. Check return code handling
3. Verify error logging

## Success Criteria

The fix is considered successful when:
- ✓ All TC1-TC10 test cases pass
- ✓ Performance tests show no degradation
- ✓ Security tests pass
- ✓ Error handling tests pass
- ✓ No regression in existing functionality
- ✓ Works across all supported Windows versions
- ✓ LDAP authentication succeeds with domain-qualified usernames

## Sign-Off

```
Test Lead: _________________ Date: _______
Developer: _________________ Date: _______
QA Manager: ________________ Date: _______
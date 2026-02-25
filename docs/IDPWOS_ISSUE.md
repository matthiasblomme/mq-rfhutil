# IDPWOS Issue: MQCSP Attached with User ID but No Password

## Problem

When a user ID is entered in RFHUtil but no password is provided, and the queue manager has
`CHCKCLNT(OPTIONAL)` configured, the connection fails with **AMQ5534E**.

### Root Cause

The original code unconditionally populated the `MQCSP` structure and set `SecurityParmsPtr`
on the `MQCNO` whenever a user ID was present — regardless of whether a password was also
supplied. Under `CHCKCLNT(OPTIONAL)`, IBM MQ only skips the authentication check when **no
`MQCSP` is attached at all**. Attaching an `MQCSP` with a non-empty user ID but an empty
password causes MQ to attempt authentication and fail.

## Fix

**File:** `RFHUtil/DataArea.cpp` — function `DataArea::connect2QM`

The MQCSP setup is now guarded so that `AuthenticationType`, `CSPPasswordPtr`,
`CSPPasswordLength`, and `SecurityParmsPtr` are only set when a **non-empty password**
accompanies the user ID.

When a user ID is present but no password is given:
- The `MQCSP` fields remain at their zero-initialised defaults.
- `SecurityParmsPtr` is **not** set on the `MQCNO`.
- The connection proceeds without an authentication challenge.

### Code Change (before → after)

**Before** — password length and pointer were set unconditionally:

```cpp
csp.CSPPasswordLength = m_conn_password.GetLength();
csp.CSPPasswordPtr    = (void *)((LPCTSTR)m_conn_password);
csp.AuthenticationType = MQCSP_AUTH_USER_ID_AND_PWD;
cno.SecurityParmsPtr   = &csp;
if (cno.Version < MQCNO_VERSION_5)
    cno.Version = MQCNO_VERSION_5;
```

**After** — MQCSP is only activated when a password is present:

```cpp
int pwdlen = m_conn_password.GetLength();
if (pwdlen > 0)
{
    csp.CSPPasswordLength  = pwdlen;
    csp.CSPPasswordPtr     = (void *)((LPCTSTR)m_conn_password);
    csp.AuthenticationType = MQCSP_AUTH_USER_ID_AND_PWD;
    cno.SecurityParmsPtr   = &csp;
    if (cno.Version < MQCNO_VERSION_5)
        cno.Version = MQCNO_VERSION_5;
}
```

## Behaviour Matrix

| User ID | Password | MQCSP attached | Expected outcome                        |
|---------|----------|----------------|-----------------------------------------|
| empty   | empty    | No             | Anonymous connect (as before)           |
| set     | empty    | No             | Connect without auth challenge (fixed)  |
| set     | set      | Yes            | Full user-ID/password authentication    |

## Affected Binaries (rebuilt)

- `bin/Release/rfhutil.exe`
- `bin/Release/rfhutilc.exe`
- `bin/ReleaseSafe/rfhutilc-safe.exe`

## Commit

`830190d` — fix: only attach MQCSP when both user ID and password are provided

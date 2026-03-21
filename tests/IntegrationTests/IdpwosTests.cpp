// IdpwosTests.cpp — integration tests for the IDPWOS fix
//
// Tests the MQCONNX behaviour with different user-ID / password combinations,
// mirroring the logic fixed in DataArea::connect2QM (RFHUtil/DataArea.cpp).
//
// Requires a live QM on localhost:1414 (or override via env vars).
// Default target: QM1 via DEV.APP.SVRCONN with MQ_APP_USER=app.
//
// Environment variables (all optional, defaults match the docker compose QM):
//   MQ_HOST      — queue manager hostname  (default: localhost)
//   MQ_PORT      — listener port           (default: 1414)
//   MQ_CHANNEL   — server-connection channel (default: DEV.APP.SVRCONN)
//   MQ_QMGR      — queue manager name     (default: QM1)
//   MQ_USER      — test user ID            (default: app)
//   MQ_PASSWORD  — correct password        (default: passw0rd)

#include <gtest/gtest.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>

// MQ headers
#include <cmqc.h>
#include <cmqxc.h>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static const char* env(const char* name, const char* def)
{
    const char* v = getenv(name);
    return v ? v : def;
}

struct ConnResult
{
    MQLONG cc;
    MQLONG rc;
};

// Build a connection name string "host(port)"
static void buildConnName(char* out, int outLen)
{
    snprintf(out, outLen, "%s(%s)",
             env("MQ_HOST", "localhost"),
             env("MQ_PORT", "1414"));
}

// Attempt MQCONNX with explicit control over whether MQCSP is attached.
// This mirrors the fixed logic in DataArea::connect2QM:
//   - user + password  → MQCSP attached (full authentication)
//   - user only        → MQCSP NOT attached (no auth challenge)
//   - neither          → MQCSP NOT attached
static ConnResult tryConnect(const char* userId, const char* password)
{
    MQCNO cno = { MQCNO_DEFAULT };
    MQCD  cd  = { MQCD_CLIENT_CONN_DEFAULT };
    MQCSP csp = { MQCSP_DEFAULT };

    // Channel
    char channel[MQ_CHANNEL_NAME_LENGTH + 1] = {};
    strncpy(channel, env("MQ_CHANNEL", "DEV.APP.SVRCONN"), MQ_CHANNEL_NAME_LENGTH);
    memcpy(cd.ChannelName, channel, strlen(channel));

    // Connection name "host(port)"
    buildConnName(cd.ConnectionName, sizeof(cd.ConnectionName));
    cd.TransportType = MQXPT_TCP;

    cno.ClientConnPtr = &cd;
    cno.Options       = MQCNO_NONE;

    int userLen = userId   ? (int)strlen(userId)   : 0;
    int pwdLen  = password ? (int)strlen(password) : 0;

    if (userLen > 0)
    {
        csp.CSPUserIdPtr    = (void*)userId;
        csp.CSPUserIdLength = userLen;

        // IDPWOS fix: only attach MQCSP when a password is also provided.
        if (pwdLen > 0)
        {
            csp.CSPPasswordPtr    = (void*)password;
            csp.CSPPasswordLength = pwdLen;
            csp.AuthenticationType = MQCSP_AUTH_USER_ID_AND_PWD;

            cno.SecurityParmsPtr = &csp;
            if (cno.Version < MQCNO_VERSION_5)
                cno.Version = MQCNO_VERSION_5;
        }
        // else: no password → do NOT set SecurityParmsPtr
    }

    MQHCONN hConn = MQHC_UNUSABLE_HCONN;
    MQLONG  cc, rc;
    MQCHAR48 qmName = {};
    strncpy(qmName, env("MQ_QMGR", "QM1"), sizeof(qmName) - 1);

    MQCONNX(qmName, &cno, &hConn, &cc, &rc);

    if (cc == MQCC_OK)
    {
        MQLONG dcc, drc;
        MQDISC(&hConn, &dcc, &drc);
    }

    return { cc, rc };
}

// ---------------------------------------------------------------------------
// Test cases
// ---------------------------------------------------------------------------

// TC1: user ID + correct password → MQCSP attached → authentication succeeds
TEST(Idpwos, TC1_UserAndPasswordSucceeds)
{
    auto result = tryConnect(env("MQ_USER", "app"), env("MQ_PASSWORD", "passw0rd"));
    EXPECT_EQ(MQCC_OK, result.cc)
        << "RC=" << result.rc
        << " — expected success with valid user+password";
}

// TC2: user ID only, no password → MQCSP NOT attached → no auth challenge → success
// This is the regression test for the IDPWOS fix (commit 830190d).
// Before the fix: MQCSP was attached with empty password → AMQ5534E (MQRC_NOT_AUTHORIZED).
// After  the fix: MQCSP is omitted → MQ skips the auth check → connection succeeds.
TEST(Idpwos, TC2_UserWithoutPasswordSucceeds)
{
    auto result = tryConnect(env("MQ_USER", "app"), "");
    EXPECT_EQ(MQCC_OK, result.cc)
        << "RC=" << result.rc
        << " — IDPWOS fix regression: user without password should succeed on CHCKCLNT(REQDADM) "
           "for non-admin users (no MQCSP attached)";
}

// TC3: no user ID, no password → anonymous connect → should succeed
TEST(Idpwos, TC3_AnonymousConnectSucceeds)
{
    auto result = tryConnect("", "");
    EXPECT_EQ(MQCC_OK, result.cc)
        << "RC=" << result.rc
        << " — anonymous connect should succeed";
}

// TC4: user ID + wrong password → MQCSP attached → auth fails
TEST(Idpwos, TC4_WrongPasswordFails)
{
    auto result = tryConnect(env("MQ_USER", "app"), "definitely_wrong_password");
    EXPECT_NE(MQCC_OK, result.cc)
        << "Expected authentication failure with wrong password";
}

// ---------------------------------------------------------------------------
// Entry point (console app, no MFC)
// ---------------------------------------------------------------------------

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

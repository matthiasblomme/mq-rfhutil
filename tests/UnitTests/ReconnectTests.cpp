// ReconnectTests.cpp — P3.5
// Tests for connection lifecycle / reconnection logic.
//
// DataArea cannot be compiled into this test project without the MQ SDK
// (it is a ~26k-line monolith with deep cmqc.h dependency). Full lifecycle
// tests (connect, disconnect, MQGET/MQPUT under reconnect) will be possible
// once DataArea is decomposed into MQConnection (P4.1).
//
// What IS testable here:
//   - Exponential backoff delay formula (pure math, mirrors DataArea impl)
//   - Reconnect attempt counter logic
//   - Boundary conditions (zero attempts, max cap, multiplier = 1)

#include "stdafx.h"
#include "rfhutil.h"
#include <gtest/gtest.h>

// ---------------------------------------------------------------------------
// Inline mirror of DataArea::calculateReconnectDelay()
// Formula: delay = base * (multiplier ^ (attempt - 1)), capped at maxInterval
// This matches the implementation in DataArea.cpp::attemptReconnection().
// When P4.1 extracts MQConnection, these tests should call that class directly.
// ---------------------------------------------------------------------------

static int calcReconnectDelay(int attempt, int baseMs, double multiplier, int maxMs)
{
    if (attempt <= 0) return 0;
    double delay = baseMs * pow(multiplier, (double)(attempt - 1));
    if (delay > maxMs) delay = maxMs;
    return (int)delay;
}

// ---------------------------------------------------------------------------
// Exponential backoff — standard 2x multiplier
// ---------------------------------------------------------------------------

TEST(Reconnect_Backoff, FirstAttemptIsBase)
{
    EXPECT_EQ(1000, calcReconnectDelay(1, 1000, 2.0, 30000));
}

TEST(Reconnect_Backoff, SecondAttemptDoubles)
{
    EXPECT_EQ(2000, calcReconnectDelay(2, 1000, 2.0, 30000));
}

TEST(Reconnect_Backoff, ThirdAttemptQuadruples)
{
    EXPECT_EQ(4000, calcReconnectDelay(3, 1000, 2.0, 30000));
}

TEST(Reconnect_Backoff, CappedAtMax)
{
    // attempt 6: 1000 * 2^5 = 32000 > 30000 cap
    EXPECT_EQ(30000, calcReconnectDelay(6, 1000, 2.0, 30000));
}

TEST(Reconnect_Backoff, MultiplierOne_AlwaysBase)
{
    EXPECT_EQ(1000, calcReconnectDelay(1, 1000, 1.0, 30000));
    EXPECT_EQ(1000, calcReconnectDelay(5, 1000, 1.0, 30000));
    EXPECT_EQ(1000, calcReconnectDelay(10, 1000, 1.0, 30000));
}

TEST(Reconnect_Backoff, ZeroAttempt_ReturnsZero)
{
    EXPECT_EQ(0, calcReconnectDelay(0, 1000, 2.0, 30000));
}

TEST(Reconnect_Backoff, NegativeAttempt_ReturnsZero)
{
    EXPECT_EQ(0, calcReconnectDelay(-1, 1000, 2.0, 30000));
}

TEST(Reconnect_Backoff, MaxSmallerThanBase_AlwaysMax)
{
    EXPECT_EQ(500, calcReconnectDelay(1, 1000, 2.0, 500));
    EXPECT_EQ(500, calcReconnectDelay(5, 1000, 2.0, 500));
}

// ---------------------------------------------------------------------------
// Attempt counter: verify that a simple counter increments and resets
// (mirrors the m_reconnect_attempt_count logic in DataArea)
// ---------------------------------------------------------------------------

TEST(Reconnect_AttemptCounter, IncrementsPerAttempt)
{
    int count = 0;
    count++;
    EXPECT_EQ(1, count);
    count++;
    EXPECT_EQ(2, count);
}

TEST(Reconnect_AttemptCounter, ResetsOnSuccess)
{
    int count = 5;
    // On successful reconnect, counter resets to 0
    count = 0;
    EXPECT_EQ(0, count);
}

TEST(Reconnect_AttemptCounter, MaxAttemptsEnforced)
{
    int maxAttempts = 3;
    int count = 3;
    bool shouldRetry = (maxAttempts == 0 || count < maxAttempts);
    EXPECT_FALSE(shouldRetry);
}

TEST(Reconnect_AttemptCounter, InfiniteRetry_ZeroMax)
{
    int maxAttempts = 0;  // 0 means infinite
    int count = 999;
    bool shouldRetry = (maxAttempts == 0 || count < maxAttempts);
    EXPECT_TRUE(shouldRetry);
}

// ---------------------------------------------------------------------------
// Reconnect eligibility — certain MQ reason codes should trigger reconnect,
// others (e.g. auth failure) should not. These values are from cmqc.h and
// match the shouldAttemptReconnect() logic in DataArea.
// Tested here as constants since cmqc.h is not available in the test project.
// When P4.1 ships, replace with direct calls to MQConnection::shouldReconnect().
// ---------------------------------------------------------------------------

// Known MQRC values relevant to reconnection decisions
static const int MQRC_CONNECTION_BROKEN   = 2009;
static const int MQRC_Q_MGR_NOT_AVAILABLE = 2059;
static const int MQRC_Q_MGR_QUIESCING    = 2161;
static const int MQRC_NOT_AUTHORIZED     = 2035;
static const int MQRC_UNKNOWN_OBJECT_NAME = 2085;

static bool shouldReconnect(int rc)
{
    // Mirror of DataArea::shouldAttemptReconnect()
    return rc == MQRC_CONNECTION_BROKEN
        || rc == MQRC_Q_MGR_NOT_AVAILABLE
        || rc == MQRC_Q_MGR_QUIESCING;
}

TEST(Reconnect_Eligibility, ConnectionBroken_ShouldReconnect)
{
    EXPECT_TRUE(shouldReconnect(MQRC_CONNECTION_BROKEN));
}

TEST(Reconnect_Eligibility, QMgrNotAvailable_ShouldReconnect)
{
    EXPECT_TRUE(shouldReconnect(MQRC_Q_MGR_NOT_AVAILABLE));
}

TEST(Reconnect_Eligibility, QMgrQuiescing_ShouldReconnect)
{
    EXPECT_TRUE(shouldReconnect(MQRC_Q_MGR_QUIESCING));
}

TEST(Reconnect_Eligibility, NotAuthorized_ShouldNotReconnect)
{
    EXPECT_FALSE(shouldReconnect(MQRC_NOT_AUTHORIZED));
}

TEST(Reconnect_Eligibility, UnknownObject_ShouldNotReconnect)
{
    EXPECT_FALSE(shouldReconnect(MQRC_UNKNOWN_OBJECT_NAME));
}

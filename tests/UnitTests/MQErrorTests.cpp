#include "prelude.h"
#include <gtest/gtest.h>
#include <string.h>
#include "mqerror.h"

TEST(MQError_KnownReason, NotAuthorized)
{
    EXPECT_STREQ("Not authorized", mqReasonString(2035));
}

TEST(MQError_KnownReason, ConnectionBroken)
{
    EXPECT_STREQ("Connection to queue manager lost", mqReasonString(2009));
}

TEST(MQError_KnownReason, NoMsgAvailable)
{
    EXPECT_STREQ("No message available", mqReasonString(2033));
}

TEST(MQError_KnownReason, QueueFull)
{
    EXPECT_STREQ("Queue full", mqReasonString(2053));
}

TEST(MQError_KnownReason, HostNotAvailable)
{
    EXPECT_STREQ("Host not available", mqReasonString(2538));
}

TEST(MQError_KnownReason, Success)
{
    EXPECT_STREQ("No error", mqReasonString(0));
}

TEST(MQError_KnownReason, AttributeLocked)
{
    EXPECT_STREQ("Attribute locked", mqReasonString(6104));
}

TEST(MQError_Unknown, ReturnsNullForUnmappedCode)
{
    EXPECT_EQ(nullptr, mqReasonString(9999));
}

TEST(MQError_Unknown, ReturnsNullForNegative)
{
    EXPECT_EQ(nullptr, mqReasonString(-1));
}

// Spot-check that all returned strings are non-empty and reasonably short
// (sanity guard against accidental empty literals or huge text blobs).
TEST(MQError_Sanity, MappedStringsAreReasonable)
{
    const long codes[] = {0, 2001, 2009, 2033, 2035, 2053, 2059, 2085,
                          2110, 2161, 2195, 2202, 2334, 2393, 2425, 2538, 6104};
    for (long rc : codes) {
        const char* s = mqReasonString(rc);
        ASSERT_NE(nullptr, s) << "rc=" << rc << " unexpectedly unmapped";
        size_t n = strlen(s);
        EXPECT_GT(n, 0u) << "rc=" << rc << " has empty description";
        EXPECT_LT(n, 128u) << "rc=" << rc << " description is suspiciously long";
    }
}

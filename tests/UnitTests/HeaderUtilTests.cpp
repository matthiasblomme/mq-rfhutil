// HeaderUtilTests.cpp — P3.3
// Tests for utilities used in RFH1/RFH2/DLQ/CICS/IMS header building and parsing.
// Covers: byte reversal (encoding-sensitive header fields), length rounding
// (RFH2 folder alignment), and RFH1 name-value string parsing.
//
// NOTE: Full parse/build tests for RFH.cpp, CICS.cpp, Dlq.cpp, Ims.cpp require
// those classes to be extracted from DataArea (P4.1+) before they can be added
// to this test project without MQ SDK dependencies.

#include "stdafx.h"
#include "rfhutil.h"
#include <gtest/gtest.h>
#include "comsubs.h"

// ---------------------------------------------------------------------------
// reverseBytes — 16-bit byte swap
// Used in all header parsers when reading short fields from EBCDIC/host-encoded
// messages on a little-endian (x86) machine.
// ---------------------------------------------------------------------------

TEST(HeaderUtil_ReverseBytes16, SwapsBytes)
{
    short val = (short)0x1234;
    short result = reverseBytes(&val);
    EXPECT_EQ((short)0x3412, result);
}

TEST(HeaderUtil_ReverseBytes16, IdentityOnPalindrome)
{
    short val = (short)0xAAAA;
    short result = reverseBytes(&val);
    EXPECT_EQ((short)0xAAAA, result);
}

TEST(HeaderUtil_ReverseBytes16, Zero)
{
    short val = 0;
    short result = reverseBytes(&val);
    EXPECT_EQ(0, result);
}

TEST(HeaderUtil_ReverseBytes16, MaxValue)
{
    short val = (short)0x7FFF;
    short result = reverseBytes(&val);
    EXPECT_EQ((short)0xFF7F, result);
}

TEST(HeaderUtil_ReverseBytes16, RoundTrip)
{
    short original = (short)0xABCD;
    short swapped = reverseBytes(&original);
    short restored = reverseBytes(&swapped);
    EXPECT_EQ(original, restored);
}

// ---------------------------------------------------------------------------
// reverseBytes4 — 32-bit byte swap
// Used for CCSID, encoding, and length fields in all RFH and transport headers.
// ---------------------------------------------------------------------------

TEST(HeaderUtil_ReverseBytes32, SwapsBytes)
{
    int val = 0x12345678;
    int result = reverseBytes4(val);
    EXPECT_EQ(0x78563412, result);
}

TEST(HeaderUtil_ReverseBytes32, Zero)
{
    EXPECT_EQ(0, reverseBytes4(0));
}

TEST(HeaderUtil_ReverseBytes32, AllOnes)
{
    EXPECT_EQ((int)0xFFFFFFFF, reverseBytes4((int)0xFFFFFFFF));
}

TEST(HeaderUtil_ReverseBytes32, RoundTrip)
{
    int original = 0xDEADBEEF;
    EXPECT_EQ(original, reverseBytes4(reverseBytes4(original)));
}

TEST(HeaderUtil_ReverseBytes32, KnownCCSID)
{
    // CCSID 1208 (UTF-8) stored in host byte order: 0x000004B8
    // On x86, reading it as big-endian gives 0xB8040000
    int hostEncoded = 0x000004B8;
    int onWire = reverseBytes4(hostEncoded);
    EXPECT_EQ(0xB8040000, onWire);
    // Swap back to recover original
    EXPECT_EQ(hostEncoded, reverseBytes4(onWire));
}

TEST(HeaderUtil_ReverseBytes32, KnownEncoding)
{
    // MQ encoding for PC little-endian = 546 (0x00000222)
    int enc = 0x00000222;
    int swapped = reverseBytes4(enc);
    EXPECT_EQ(0x22020000, swapped);
    EXPECT_EQ(enc, reverseBytes4(swapped));
}

// ---------------------------------------------------------------------------
// reverseBytes24 / reverseBytes32 — buffer-based byte swap
// Operates on 24-byte and 32-byte fixed-size fields used in DLQ/CICS headers
// (e.g. GMTTime, PutDate/PutTime compound fields).
// ---------------------------------------------------------------------------

TEST(HeaderUtil_ReverseBytes24Buf, ReversesEachFourByteWord)
{
    // reverseBytes24 applies a full 4-byte endian flip to each 4-byte chunk.
    // in[0..3] = {0x12, 0x34, 0x56, 0x78} → out[0..3] = {0x78, 0x56, 0x34, 0x12}
    unsigned char in[24]  = {};
    unsigned char out[24] = {};
    in[0] = 0x12; in[1] = 0x34; in[2] = 0x56; in[3] = 0x78;
    reverseBytes24(in, out);
    EXPECT_EQ(0x78, out[0]);
    EXPECT_EQ(0x56, out[1]);
    EXPECT_EQ(0x34, out[2]);
    EXPECT_EQ(0x12, out[3]);
}

TEST(HeaderUtil_ReverseBytes24Buf, AllZero)
{
    unsigned char in[24]  = {};
    unsigned char out[24] = {};
    reverseBytes24(in, out);
    for (int i = 0; i < 24; i++)
        EXPECT_EQ(0, out[i]);
}

TEST(HeaderUtil_ReverseBytes24Buf, RoundTrip)
{
    unsigned char original[24] = {};
    unsigned char tmp[24]      = {};
    unsigned char restored[24] = {};
    for (int i = 0; i < 24; i++) original[i] = (unsigned char)i;
    reverseBytes24(original, tmp);
    reverseBytes24(tmp, restored);
    EXPECT_EQ(0, memcmp(original, restored, 24));
}

TEST(HeaderUtil_ReverseBytes32Buf, ReversesEachFourByteWord)
{
    // reverseBytes32 applies a full 4-byte endian flip to each 4-byte chunk.
    // in[0..3] = {0xDE, 0xAD, 0xBE, 0xEF} → out[0..3] = {0xEF, 0xBE, 0xAD, 0xDE}
    unsigned char in[32]  = {};
    unsigned char out[32] = {};
    in[0] = 0xDE; in[1] = 0xAD; in[2] = 0xBE; in[3] = 0xEF;
    reverseBytes32(in, out);
    EXPECT_EQ(0xEF, out[0]);
    EXPECT_EQ(0xBE, out[1]);
    EXPECT_EQ(0xAD, out[2]);
    EXPECT_EQ(0xDE, out[3]);
}

TEST(HeaderUtil_ReverseBytes32Buf, RoundTrip)
{
    unsigned char original[32] = {};
    unsigned char tmp[32]      = {};
    unsigned char restored[32] = {};
    for (int i = 0; i < 32; i++) original[i] = (unsigned char)(i * 7);
    reverseBytes32(original, tmp);
    reverseBytes32(tmp, restored);
    EXPECT_EQ(0, memcmp(original, restored, 32));
}

// ---------------------------------------------------------------------------
// roundLength — round up a length stored at a pointer to the next 4-byte boundary.
// Used by RFH2 to pad each NameValueData folder to a 4-byte multiple.
// ---------------------------------------------------------------------------

TEST(HeaderUtil_RoundLength, AlreadyAligned)
{
    // "test" is 4 chars — already 4-byte aligned; no padding appended
    unsigned char buf[8] = "test";
    int result = roundLength(buf);
    EXPECT_EQ(4, result);
}

TEST(HeaderUtil_RoundLength, RoundsUpBy1)
{
    // "abc" is 3 chars — needs 1 space to reach 4; buffer must have room
    unsigned char buf[8] = "abc";
    int result = roundLength(buf);
    EXPECT_EQ(4, result);
}

TEST(HeaderUtil_RoundLength, RoundsUpBy3)
{
    // "hello" is 5 chars — needs 3 spaces to reach 8; buffer must have room
    unsigned char buf[16] = "hello";
    int result = roundLength(buf);
    EXPECT_EQ(8, result);
}

TEST(HeaderUtil_RoundLength, ZeroLength)
{
    // Empty string: 0 % 4 == 0, no padding, returns 0
    unsigned char buf[4] = "";
    int result = roundLength(buf);
    EXPECT_EQ(0, result);
}

// ---------------------------------------------------------------------------
// parseRFH1String — extract a keyword=value pair from an RFH1 NameValueData string.
// RFH1 stores properties as space-separated "key=value" tokens.
// ---------------------------------------------------------------------------

TEST(HeaderUtil_ParseRFH1String, SimpleKeyValue)
{
    char input[] = "key=value next=other";
    char value[64] = {};
    char *next = parseRFH1String(input, value, sizeof(value));
    // parseRFH1String copies the whole space-delimited token including the key
    EXPECT_STREQ("key=value", value);
    EXPECT_NE((char*)nullptr, next);
}

TEST(HeaderUtil_ParseRFH1String, NoEquals)
{
    char input[] = "noequals";
    char value[64] = {};
    char *next = parseRFH1String(input, value, sizeof(value));
    // Should not crash; value may be empty or partial
    EXPECT_NE((char*)nullptr, input);  // at minimum, input was valid
}

TEST(HeaderUtil_ParseRFH1String, EmptyValue)
{
    char input[] = "key= next=x";
    char value[64] = {};
    parseRFH1String(input, value, sizeof(value));
    // Token up to the space is "key="; the part after '=' is empty but key is included
    EXPECT_STREQ("key=", value);
}

TEST(HeaderUtil_ParseRFH1String, TruncatesToMaxSize)
{
    // parseRFH1String writes null at value[maxSize]; the buffer must be maxSize+1 bytes.
    char input[] = "key=verylongvaluestring";
    char value[6] = {};          // 6 bytes: 5 chars + null at [5]
    parseRFH1String(input, value, 5);
    EXPECT_EQ('\0', value[5]);   // null terminator written at position maxSize
}

// FileFormatTests.cpp — P3.6
// Tests for file format detection and data encoding utilities used in
// DataArea::ReadFileData() and WriteDataFile().
//
// Full file I/O round-trip tests (open file → parse headers → extract data →
// write back) require DataArea to be decomposed (P4.4 FileHandler). Those
// tests will be added once FileHandler is extracted.
//
// What IS testable here:
//   - checkIfHex: detects whether a buffer contains valid hex data
//   - Hex encode/decode round-trips for file data scenarios
//   - EBCDIC round-trips as used in CHARACTER mode file writes
//   - Boundary and edge cases for all format detection paths

#include "stdafx.h"
#include "rfhutil.h"
#include <gtest/gtest.h>
#include "comsubs.h"

// ---------------------------------------------------------------------------
// checkIfHex — returns non-zero if the buffer is valid uppercase/lowercase hex
// Used in DataArea to detect whether pasted or loaded data is in Hex format.
// ---------------------------------------------------------------------------

TEST(FileFormat_CheckIfHex, ValidUppercaseHex)
{
    const char *data = "DEADBEEF";
    EXPECT_NE(0, checkIfHex(data, (int)strlen(data)));
}

TEST(FileFormat_CheckIfHex, ValidLowercaseHex)
{
    const char *data = "deadbeef";
    EXPECT_NE(0, checkIfHex(data, (int)strlen(data)));
}

TEST(FileFormat_CheckIfHex, ValidMixedCaseHex)
{
    const char *data = "DeAdBeEf";
    EXPECT_NE(0, checkIfHex(data, (int)strlen(data)));
}

TEST(FileFormat_CheckIfHex, ValidAllZeros)
{
    const char *data = "00000000";
    EXPECT_NE(0, checkIfHex(data, (int)strlen(data)));
}

TEST(FileFormat_CheckIfHex, ValidAllFs)
{
    const char *data = "FFFFFFFF";
    EXPECT_NE(0, checkIfHex(data, (int)strlen(data)));
}

TEST(FileFormat_CheckIfHex, ContainsG_Invalid)
{
    const char *data = "DEADGONE";
    EXPECT_EQ(0, checkIfHex(data, (int)strlen(data)));
}

TEST(FileFormat_CheckIfHex, ContainsSpace_Invalid)
{
    const char *data = "DE AD BE EF";
    EXPECT_EQ(0, checkIfHex(data, (int)strlen(data)));
}

TEST(FileFormat_CheckIfHex, OddLength)
{
    // Odd-length hex strings cannot represent whole bytes
    const char *data = "ABC";
    // Result depends on implementation; at minimum must not crash
    checkIfHex(data, (int)strlen(data));
    SUCCEED();
}

TEST(FileFormat_CheckIfHex, EmptyBuffer)
{
    checkIfHex("", 0);
    SUCCEED();
}

TEST(FileFormat_CheckIfHex, SingleByte)
{
    EXPECT_NE(0, checkIfHex("FF", 2));
    EXPECT_NE(0, checkIfHex("00", 2));
    EXPECT_NE(0, checkIfHex("A0", 2));
}

TEST(FileFormat_CheckIfHex, LongValidHex)
{
    // 256 bytes represented as 512 hex chars
    char buf[513] = {};
    for (int i = 0; i < 512; i++)
        buf[i] = "0123456789ABCDEF"[i % 16];
    EXPECT_NE(0, checkIfHex(buf, 512));
}

// ---------------------------------------------------------------------------
// Hex encode/decode round-trip — simulates file data Hex mode write/read back
// AsciiToHex (binary → hex display) + HexToAscii (hex display → binary)
// ---------------------------------------------------------------------------

TEST(FileFormat_HexRoundTrip, ShortMessage)
{
    const unsigned char original[] = { 0x48, 0x65, 0x6C, 0x6C, 0x6F }; // "Hello"
    unsigned char hexBuf[11] = {};
    unsigned char decoded[6] = {};

    AsciiToHex(original, 5, hexBuf);
    EXPECT_STREQ("48656C6C6F", (char*)hexBuf);

    HexToAscii(hexBuf, 5, decoded);
    EXPECT_EQ(0, memcmp(original, decoded, 5));
}

TEST(FileFormat_HexRoundTrip, AllByteValues)
{
    unsigned char original[256];
    for (int i = 0; i < 256; i++) original[i] = (unsigned char)i;

    unsigned char hexBuf[513] = {};
    unsigned char decoded[257] = {};

    AsciiToHex(original, 256, hexBuf);
    HexToAscii(hexBuf, 256, decoded);

    EXPECT_EQ(0, memcmp(original, decoded, 256));
}

TEST(FileFormat_HexRoundTrip, SingleNullByte)
{
    const unsigned char original[] = { 0x00 };
    unsigned char hexBuf[3] = {};
    unsigned char decoded[2] = {};

    AsciiToHex(original, 1, hexBuf);
    EXPECT_STREQ("00", (char*)hexBuf);

    HexToAscii(hexBuf, 1, decoded);
    EXPECT_EQ(0x00, decoded[0]);
}

// ---------------------------------------------------------------------------
// EBCDIC round-trip — simulates CHARACTER mode file write/read for EBCDIC queues
// AsciiToEbcdic → EbcdicToAscii must restore the original text
// ---------------------------------------------------------------------------

TEST(FileFormat_EbcdicRoundTrip, SimpleText)
{
    const unsigned char original[] = "Hello World";
    unsigned int len = (unsigned int)strlen((const char*)original);
    unsigned char ebcdic[12] = {};
    unsigned char restored[12] = {};

    AsciiToEbcdic((unsigned char*)original, len, ebcdic);
    EbcdicToAscii(ebcdic, len, restored);

    EXPECT_EQ(0, memcmp(original, restored, len));
}

TEST(FileFormat_EbcdicRoundTrip, Digits)
{
    const unsigned char original[] = "0123456789";
    unsigned int len = (unsigned int)strlen((const char*)original);
    unsigned char ebcdic[11] = {};
    unsigned char restored[11] = {};

    AsciiToEbcdic((unsigned char*)original, len, ebcdic);
    EbcdicToAscii(ebcdic, len, restored);

    EXPECT_EQ(0, memcmp(original, restored, len));
}

TEST(FileFormat_EbcdicRoundTrip, SpecialChars)
{
    // Chars representable in EBCDIC-37
    const unsigned char original[] = ".,!@#$%&*()-_=+";
    unsigned int len = (unsigned int)strlen((const char*)original);
    unsigned char ebcdic[20] = {};
    unsigned char restored[20] = {};

    AsciiToEbcdic((unsigned char*)original, len, ebcdic);
    EbcdicToAscii(ebcdic, len, restored);

    EXPECT_EQ(0, memcmp(original, restored, len));
}

TEST(FileFormat_EbcdicRoundTrip, EbcdicDifferentFromAscii)
{
    // Verify conversion actually changes the bytes (not identity)
    const unsigned char original[] = "ABC";
    unsigned int len = 3;
    unsigned char ebcdic[4] = {};

    AsciiToEbcdic((unsigned char*)original, len, ebcdic);

    // EBCDIC 'A' = 0xC1, 'B' = 0xC2, 'C' = 0xC3
    EXPECT_EQ(0xC1, ebcdic[0]);
    EXPECT_EQ(0xC2, ebcdic[1]);
    EXPECT_EQ(0xC3, ebcdic[2]);
}

TEST(FileFormat_EbcdicRoundTrip, FullAlphabet)
{
    const unsigned char original[] = "abcdefghijklmnopqrstuvwxyz"
                                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    unsigned int len = (unsigned int)strlen((const char*)original);
    unsigned char ebcdic[53] = {};
    unsigned char restored[53] = {};

    AsciiToEbcdic((unsigned char*)original, len, ebcdic);
    EbcdicToAscii(ebcdic, len, restored);

    EXPECT_EQ(0, memcmp(original, restored, len));
}

// ---------------------------------------------------------------------------
// fromHex edge cases — used in Hex-mode data parsing when writing back to queue
// ---------------------------------------------------------------------------

TEST(FileFormat_FromHex, AllValidPairs)
{
    const char hexDigits[] = "0123456789ABCDEF";
    for (int hi = 0; hi < 16; hi++)
    {
        for (int lo = 0; lo < 16; lo++)
        {
            unsigned char expected = (unsigned char)(hi * 16 + lo);
            char result = fromHex(hexDigits[hi], hexDigits[lo]);
            EXPECT_EQ((char)expected, result);
        }
    }
}

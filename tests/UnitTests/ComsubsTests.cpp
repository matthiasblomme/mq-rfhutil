// ComsubsTests.cpp — unit tests for RFHUtil/comsubs.cpp
// Covers: hex encoding, EBCDIC conversion, string utilities, byte reversal,
//         integer parsing.

#include "stdafx.h"
#include "rfhutil.h"
#include <gtest/gtest.h>
#include "comsubs.h"

// ---------------------------------------------------------------------------
// Hex encoding
// ---------------------------------------------------------------------------

TEST(ComsubsHex, AsciiToHexBasic)
{
    const unsigned char input[] = { 0xDE, 0xAD, 0xBE, 0xEF };
    unsigned char output[9] = {};
    AsciiToHex(input, 4, output);
    EXPECT_STREQ("DEADBEEF", (char*)output);
}

TEST(ComsubsHex, AsciiToHexAllZero)
{
    const unsigned char input[] = { 0x00 };
    unsigned char output[3] = {};
    AsciiToHex(input, 1, output);
    EXPECT_STREQ("00", (char*)output);
}

TEST(ComsubsHex, HexToAsciiBasic)
{
    unsigned char input[] = "DEADBEEF";
    unsigned char output[5] = {};
    HexToAscii(input, 8, output);
    EXPECT_EQ(0xDE, output[0]);
    EXPECT_EQ(0xAD, output[1]);
    EXPECT_EQ(0xBE, output[2]);
    EXPECT_EQ(0xEF, output[3]);
}

TEST(ComsubsHex, HexRoundTrip)
{
    const unsigned char original[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
    unsigned char hexBuf[17] = {};
    unsigned char decoded[9] = {};

    AsciiToHex(original, 8, hexBuf);
    HexToAscii(hexBuf, 16, decoded);

    EXPECT_EQ(0, memcmp(original, decoded, 8));
}

TEST(ComsubsHex, CheckIfHexValid)
{
    EXPECT_NE(0, checkIfHex("0123456789ABCDEF", 16));
    EXPECT_NE(0, checkIfHex("abcdef",           6));
    EXPECT_NE(0, checkIfHex("FF",               2));
}

TEST(ComsubsHex, CheckIfHexInvalid)
{
    EXPECT_EQ(0, checkIfHex("GGGG",    4));
    EXPECT_EQ(0, checkIfHex("Hello",   5));
    EXPECT_EQ(0, checkIfHex("12 34",   5));  // space is not hex
}

TEST(ComsubsHex, CharValueDigits)
{
    for (char c = '0'; c <= '9'; c++)
        EXPECT_EQ(c - '0', charValue(c));
}

TEST(ComsubsHex, CharValueUpperHex)
{
    EXPECT_EQ(10, charValue('A'));
    EXPECT_EQ(11, charValue('B'));
    EXPECT_EQ(15, charValue('F'));
}

TEST(ComsubsHex, CharValueLowerHex)
{
    EXPECT_EQ(10, charValue('a'));
    EXPECT_EQ(15, charValue('f'));
}

TEST(ComsubsHex, FromHex)
{
    EXPECT_EQ((char)0xAB, fromHex('A', 'B'));
    EXPECT_EQ((char)0x00, fromHex('0', '0'));
    EXPECT_EQ((char)0xFF, fromHex('F', 'F'));
    EXPECT_EQ((char)0x10, fromHex('1', '0'));
}

// ---------------------------------------------------------------------------
// EBCDIC conversion
// ---------------------------------------------------------------------------

TEST(ComsubsEbcdic, AsciiToEbcdicRoundTrip)
{
    // Printable ASCII 0x20–0x7E survives a round-trip
    unsigned char original[95];
    for (int i = 0; i < 94; i++) original[i] = (unsigned char)(0x20 + i);
    original[94] = '\0';

    unsigned char ebcdic[95] = {};
    unsigned char back[95]   = {};

    AsciiToEbcdic(original, 94, ebcdic);
    EbcdicToAscii(ebcdic,   94, back);

    EXPECT_EQ(0, memcmp(original, back, 94));
}

TEST(ComsubsEbcdic, KnownMappings)
{
    // Space: ASCII 0x20 → EBCDIC 0x40
    unsigned char sp = 0x20, out = 0;
    AsciiToEbcdic(&sp, 1, &out);
    EXPECT_EQ(0x40, out);

    // 'A': ASCII 0x41 → EBCDIC 0xC1
    unsigned char a = 'A';
    AsciiToEbcdic(&a, 1, &out);
    EXPECT_EQ(0xC1, out);

    // '0': ASCII 0x30 → EBCDIC 0xF0
    unsigned char zero = '0';
    AsciiToEbcdic(&zero, 1, &out);
    EXPECT_EQ(0xF0, out);
}

TEST(ComsubsEbcdic, EbcdicToAsciiKnown)
{
    // EBCDIC 0x40 (space) → ASCII 0x20
    const unsigned char sp = 0x40;
    unsigned char out = 0;
    EbcdicToAscii(&sp, 1, &out);
    EXPECT_EQ(0x20, out);

    // EBCDIC 0xC1 ('A') → ASCII 0x41
    const unsigned char a = 0xC1;
    EbcdicToAscii(&a, 1, &out);
    EXPECT_EQ('A', out);
}

// ---------------------------------------------------------------------------
// String utilities
// ---------------------------------------------------------------------------

TEST(ComsubsString, SkipWhiteSpaceLeading)
{
    const char *str = "   hello";
    const char *end = str + strlen(str);
    const char *p   = skipWhiteSpace(str, end);
    EXPECT_EQ('h', *p);
}

TEST(ComsubsString, SkipWhiteSpaceNone)
{
    const char *str = "hello";
    const char *end = str + strlen(str);
    EXPECT_EQ(str, skipWhiteSpace(str, end));
}

TEST(ComsubsString, SkipWhiteSpaceAllSpaces)
{
    const char *str = "   ";
    const char *end = str + strlen(str);
    EXPECT_EQ(end, skipWhiteSpace(str, end));
}

TEST(ComsubsString, SkipBlanks)
{
    const char *str = "   world";
    EXPECT_EQ('w', *skipBlanks(str));
}

TEST(ComsubsString, SkipBlanksNoLeading)
{
    const char *str = "world";
    EXPECT_EQ(str, skipBlanks(str));
}

TEST(ComsubsString, RtrimTrailing)
{
    char buf[] = "hello   ";
    Rtrim(buf);
    EXPECT_STREQ("hello", buf);
}

TEST(ComsubsString, RtrimNone)
{
    char buf[] = "hello";
    Rtrim(buf);
    EXPECT_STREQ("hello", buf);
}

TEST(ComsubsString, RtrimAllSpaces)
{
    char buf[] = "   ";
    Rtrim(buf);
    EXPECT_STREQ("", buf);
}

// ---------------------------------------------------------------------------
// Integer parsing
// ---------------------------------------------------------------------------

TEST(ComsubsInt, MyAtoi64Positive)
{
    EXPECT_EQ(12345, my_atoi64("12345"));
}

TEST(ComsubsInt, MyAtoi64Negative)
{
    EXPECT_EQ(-42, my_atoi64("-42"));
}

TEST(ComsubsInt, MyAtoi64Zero)
{
    EXPECT_EQ(0, my_atoi64("0"));
}

TEST(ComsubsInt, MyAtoi64Large)
{
    EXPECT_EQ(9999999999LL, my_atoi64("9999999999"));
}

// ---------------------------------------------------------------------------
// Byte reversal
// ---------------------------------------------------------------------------

TEST(ComsubsReverse, ReverseBytes)
{
    short val = 0x1234;
    short result = reverseBytes(&val);
    EXPECT_EQ((short)0x3412, result);
}

TEST(ComsubsReverse, ReverseBytes4)
{
    EXPECT_EQ(0x78563412, reverseBytes4(0x12345678));
}

TEST(ComsubsReverse, ReverseBytes4Identity)
{
    // Reversing twice gives original
    int original = 0xAABBCCDD;
    EXPECT_EQ(original, reverseBytes4(reverseBytes4(original)));
}

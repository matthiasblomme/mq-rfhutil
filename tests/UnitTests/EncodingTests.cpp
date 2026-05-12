// EncodingTests.cpp — P3.4
// Tests for message encoding functions in comsubs.cpp.
// Covers: integer parsing, string utilities, UCS-2 detection, line-ending
// detection, whitespace handling, quoted-string skipping, hex digit parsing.

#include "stdafx.h"
#include "rfhutil.h"
#include <gtest/gtest.h>
#include "comsubs.h"

// ---------------------------------------------------------------------------
// my_atoi64 — parse a 64-bit signed integer from a string
// ---------------------------------------------------------------------------

TEST(Encoding_Atoi64, PositiveNumber)
{
    EXPECT_EQ(12345LL, my_atoi64("12345"));
}

TEST(Encoding_Atoi64, NegativeNumber)
{
    EXPECT_EQ(-99LL, my_atoi64("-99"));
}

TEST(Encoding_Atoi64, Zero)
{
    EXPECT_EQ(0LL, my_atoi64("0"));
}

TEST(Encoding_Atoi64, LargeValue)
{
    EXPECT_EQ(9999999999LL, my_atoi64("9999999999"));
}

TEST(Encoding_Atoi64, LeadingSpaces)
{
    // my_atoi64 breaks on any non-digit, non-minus char (including spaces); returns 0
    EXPECT_EQ(0LL, my_atoi64("   42"));
}

TEST(Encoding_Atoi64, EmptyString)
{
    EXPECT_EQ(0LL, my_atoi64(""));
}

// ---------------------------------------------------------------------------
// isUCS2 — detect UCS-2 / UTF-16 CCSIDs
// Used in header parsers to select the correct string decoding path.
// ---------------------------------------------------------------------------

TEST(Encoding_IsUCS2, Ccsid1200_IsUCS2)
{
    EXPECT_TRUE(isUCS2(1200));   // UTF-16
}

TEST(Encoding_IsUCS2, Ccsid1201_IsUCS2)
{
    EXPECT_TRUE(isUCS2(1201));   // UTF-16BE
}

TEST(Encoding_IsUCS2, Ccsid13488_IsUCS2)
{
    EXPECT_TRUE(isUCS2(13488));
}

TEST(Encoding_IsUCS2, Ccsid17584_IsUCS2)
{
    EXPECT_TRUE(isUCS2(17584));
}

TEST(Encoding_IsUCS2, Ccsid1208_NotUCS2)
{
    EXPECT_FALSE(isUCS2(1208));  // UTF-8
}

TEST(Encoding_IsUCS2, Ccsid37_NotUCS2)
{
    EXPECT_FALSE(isUCS2(37));    // EBCDIC-37
}

TEST(Encoding_IsUCS2, Ccsid0_NotUCS2)
{
    EXPECT_FALSE(isUCS2(0));
}

// ---------------------------------------------------------------------------
// findcrlf — locate the first CR (0x0D) or LF (0x0A) in a buffer
// Returns the position of the first line ending or maxchar if not found.
// Used in file I/O to detect line endings in text-format messages.
// ---------------------------------------------------------------------------

TEST(Encoding_FindCrLf, FindsLF)
{
    const unsigned char data[] = "hello\nworld";
    int pos = findcrlf(data, (int)sizeof(data) - 1);
    EXPECT_EQ(5, pos);
}

TEST(Encoding_FindCrLf, FindsCR)
{
    const unsigned char data[] = "hello\rworld";
    int pos = findcrlf(data, (int)sizeof(data) - 1);
    EXPECT_EQ(5, pos);
}

TEST(Encoding_FindCrLf, FindsCRLF_ReturnsCR)
{
    const unsigned char data[] = "ab\r\ncd";
    int pos = findcrlf(data, (int)sizeof(data) - 1);
    EXPECT_EQ(2, pos);
}

TEST(Encoding_FindCrLf, NoCrLf_ReturnsMaxChar)
{
    const unsigned char data[] = "noeol";
    int maxchar = (int)sizeof(data) - 1;
    int pos = findcrlf(data, maxchar);
    EXPECT_EQ(maxchar, pos);
}

TEST(Encoding_FindCrLf, AtStart)
{
    const unsigned char data[] = "\nhello";
    int pos = findcrlf(data, (int)sizeof(data) - 1);
    EXPECT_EQ(0, pos);
}

TEST(Encoding_FindCrLf, EmptyBuffer)
{
    const unsigned char data[] = "";
    int pos = findcrlf(data, 0);
    EXPECT_EQ(0, pos);
}

// ---------------------------------------------------------------------------
// skipWhiteSpace — advance past spaces and tabs
// Used in XML/RFH1 parsers to skip formatting whitespace.
// ---------------------------------------------------------------------------

TEST(Encoding_SkipWhiteSpace, SkipsSpaces)
{
    const char *s = "   hello";
    const char *end = s + strlen(s);
    const char *result = skipWhiteSpace(s, end);
    EXPECT_EQ('h', *result);
}

TEST(Encoding_SkipWhiteSpace, SkipsTabs)
{
    const char *s = "\t\thello";
    const char *end = s + strlen(s);
    const char *result = skipWhiteSpace(s, end);
    EXPECT_EQ('h', *result);
}

TEST(Encoding_SkipWhiteSpace, MixedWhitespace)
{
    const char *s = " \t \t x";
    const char *end = s + strlen(s);
    const char *result = skipWhiteSpace(s, end);
    EXPECT_EQ('x', *result);
}

TEST(Encoding_SkipWhiteSpace, NoWhitespace)
{
    const char *s = "hello";
    const char *end = s + strlen(s);
    const char *result = skipWhiteSpace(s, end);
    EXPECT_EQ(s, result);
}

TEST(Encoding_SkipWhiteSpace, AllWhitespace)
{
    const char *s = "   ";
    const char *end = s + strlen(s);
    const char *result = skipWhiteSpace(s, end);
    EXPECT_EQ(end, result);
}

// ---------------------------------------------------------------------------
// skipBlanks — skip leading spaces in a C-string
// ---------------------------------------------------------------------------

TEST(Encoding_SkipBlanks, SkipsLeadingSpaces)
{
    char s[] = "   hello";
    char *result = skipBlanks(s);
    EXPECT_EQ('h', *result);
}

TEST(Encoding_SkipBlanks, NoLeadingSpaces)
{
    char s[] = "hello";
    char *result = skipBlanks(s);
    EXPECT_STREQ("hello", result);
}

TEST(Encoding_SkipBlanks, AllSpaces)
{
    char s[] = "    ";
    char *result = skipBlanks(s);
    EXPECT_EQ('\0', *result);
}

// ---------------------------------------------------------------------------
// Rtrim — remove trailing spaces in-place
// ---------------------------------------------------------------------------

TEST(Encoding_Rtrim, RemovesTrailingSpaces)
{
    char s[] = "hello   ";
    Rtrim(s);
    EXPECT_STREQ("hello", s);
}

TEST(Encoding_Rtrim, NoTrailingSpaces)
{
    char s[] = "hello";
    Rtrim(s);
    EXPECT_STREQ("hello", s);
}

TEST(Encoding_Rtrim, AllSpaces)
{
    char s[] = "    ";
    Rtrim(s);
    EXPECT_STREQ("", s);
}

TEST(Encoding_Rtrim, EmptyString)
{
    char s[] = "";
    Rtrim(s);
    EXPECT_STREQ("", s);
}

TEST(Encoding_Rtrim, PreservesInternalSpaces)
{
    char s[] = "hello world  ";
    Rtrim(s);
    EXPECT_STREQ("hello world", s);
}

// ---------------------------------------------------------------------------
// findBlank — find first space or NUL
// ---------------------------------------------------------------------------

TEST(Encoding_FindBlank, FindsSpace)
{
    char s[] = "hello world";
    char *result = findBlank(s);
    EXPECT_EQ(' ', *result);
}

TEST(Encoding_FindBlank, NoSpaceReturnsNul)
{
    char s[] = "hello";
    char *result = findBlank(s);
    EXPECT_EQ('\0', *result);
}

TEST(Encoding_FindBlank, SpaceAtStart)
{
    char s[] = " hello";
    char *result = findBlank(s);
    EXPECT_EQ(s, result);
}

// ---------------------------------------------------------------------------
// skipQuotedString — skip past a quoted or unquoted token
// Used in RFH2 name-value pair parsing to step over property values.
// ---------------------------------------------------------------------------

TEST(Encoding_SkipQuotedString, UnquotedToken)
{
    const char *s   = "hello world";
    const char *end = s + strlen(s);
    const char *result = skipQuotedString(s, end);
    // Should stop at the space
    EXPECT_EQ(' ', *result);
}

TEST(Encoding_SkipQuotedString, QuotedToken)
{
    const char *s   = "'hello world' next";
    const char *end = s + strlen(s);
    const char *result = skipQuotedString(s, end);
    // Should skip past the closing quote
    EXPECT_NE(s, result);
    EXPECT_LE(result, end);
}

TEST(Encoding_SkipQuotedString, EmptyQuoted)
{
    const char *s   = "'' next";
    const char *end = s + strlen(s);
    const char *result = skipQuotedString(s, end);
    EXPECT_NE(s, result);
}

// ---------------------------------------------------------------------------
// charValue / getHexCharValue — single hex digit parsing
// ---------------------------------------------------------------------------

TEST(Encoding_CharValue, DigitZero)
{
    EXPECT_EQ(0, charValue('0'));
}

TEST(Encoding_CharValue, DigitNine)
{
    EXPECT_EQ(9, charValue('9'));
}

TEST(Encoding_CharValue, UpperA)
{
    EXPECT_EQ(10, charValue('A'));
}

TEST(Encoding_CharValue, UpperF)
{
    EXPECT_EQ(15, charValue('F'));
}

TEST(Encoding_CharValue, LowerA)
{
    EXPECT_EQ(0, charValue('a'));  // charValue handles uppercase A-F only; lowercase returns 0
}

TEST(Encoding_CharValue, LowerF)
{
    EXPECT_EQ(0, charValue('f'));  // charValue handles uppercase A-F only; lowercase returns 0
}

TEST(Encoding_GetHexCharValue, ReturnsNibble)
{
    EXPECT_EQ(0x0A, getHexCharValue('A'));
    EXPECT_EQ(0x0F, getHexCharValue('F'));
    EXPECT_EQ(0x00, getHexCharValue('0'));
    EXPECT_EQ(0x09, getHexCharValue('9'));
}

// ---------------------------------------------------------------------------
// fromHex — combine two hex chars into one byte
// ---------------------------------------------------------------------------

TEST(Encoding_FromHex, BasicConversion)
{
    EXPECT_EQ((char)0xDE, fromHex('D', 'E'));
    EXPECT_EQ((char)0xAD, fromHex('A', 'D'));
    EXPECT_EQ((char)0x00, fromHex('0', '0'));
    EXPECT_EQ((char)0xFF, fromHex('F', 'F'));
}

TEST(Encoding_FromHex, LowerCase)
{
    // charValue handles uppercase only; lowercase nybbles contribute 0
    EXPECT_EQ((char)0x00, fromHex('b', 'e'));
    EXPECT_EQ((char)0x00, fromHex('e', 'f'));
}

TEST(Encoding_FromHex, MixedCase)
{
    // Uppercase nybble decoded correctly; lowercase nybble contributes 0
    EXPECT_EQ((char)0xA0, fromHex('A', 'b'));
    EXPECT_EQ((char)0xB0, fromHex('B', 'e'));
}

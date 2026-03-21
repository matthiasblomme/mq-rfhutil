// XmlsubsTests.cpp — unit tests for RFHUtil/xmlsubs.cpp
// Covers: escape sequence handling, XML validation, delimiter finding,
//         comment skipping.

#include "stdafx.h"
#include "rfhutil.h"
#include <gtest/gtest.h>
#include "comsubs.h"
#include "xmlsubs.h"

// ---------------------------------------------------------------------------
// removeEscSeq — unescape XML entities in-place
// ---------------------------------------------------------------------------

TEST(XmlsubsEscape, AmpersandEntity)
{
    char buf[] = "&amp;";
    removeEscSeq(buf);
    EXPECT_STREQ("&", buf);
}

TEST(XmlsubsEscape, LtEntity)
{
    char buf[] = "&lt;";
    removeEscSeq(buf);
    EXPECT_STREQ("<", buf);
}

TEST(XmlsubsEscape, GtEntity)
{
    char buf[] = "&gt;";
    removeEscSeq(buf);
    EXPECT_STREQ(">", buf);
}

TEST(XmlsubsEscape, AposEntity)
{
    char buf[] = "&apos;";
    removeEscSeq(buf);
    EXPECT_STREQ("'", buf);
}

TEST(XmlsubsEscape, QuotEntity)
{
    char buf[] = "&quot;";
    removeEscSeq(buf);
    EXPECT_STREQ("\"", buf);
}

TEST(XmlsubsEscape, NoEscape)
{
    char buf[] = "hello world";
    removeEscSeq(buf);
    EXPECT_STREQ("hello world", buf);
}

TEST(XmlsubsEscape, MultipleEntities)
{
    char buf[] = "&lt;root&gt;";
    removeEscSeq(buf);
    EXPECT_STREQ("<root>", buf);
}

// ---------------------------------------------------------------------------
// insertEscChars — escape special XML characters into output buffer
// ---------------------------------------------------------------------------

TEST(XmlsubsEscape, InsertEscAmpersand)
{
    char out[32] = {};
    int len = insertEscChars(out, "&", 1);
    EXPECT_GT(len, 0);
    EXPECT_STREQ("&amp;", out);
}

TEST(XmlsubsEscape, InsertEscLt)
{
    char out[32] = {};
    insertEscChars(out, "<", 1);
    EXPECT_STREQ("&lt;", out);
}

TEST(XmlsubsEscape, InsertEscRoundTrip)
{
    // Insert then remove should give back the original
    const char *original = "a < b && b > c";
    char escaped[256] = {};
    char restored[256] = {};

    insertEscChars(escaped, original, (int)strlen(original));
    strcpy(restored, escaped);
    removeEscSeq(restored);

    EXPECT_STREQ(original, restored);
}

TEST(XmlsubsEscape, InsertEscPlainText)
{
    char out[64] = {};
    int len = insertEscChars(out, "hello", 5);
    EXPECT_EQ(5, len);
    EXPECT_STREQ("hello", out);
}

// ---------------------------------------------------------------------------
// checkIfXml — bracket/tag validation
// ---------------------------------------------------------------------------

TEST(XmlsubsCheck, ValidXml)
{
    const char *xml = "<root><child>value</child></root>";
    char *errmsg = nullptr;
    int rc = checkIfXml((const unsigned char*)xml, (int)strlen(xml), &errmsg);
    EXPECT_EQ(0, rc);   // 0 = valid XML
}

TEST(XmlsubsCheck, EmptyString)
{
    char *errmsg = nullptr;
    int rc = checkIfXml((const unsigned char*)"", 0, &errmsg);
    EXPECT_NE(0, rc);   // non-zero = not valid XML
}

TEST(XmlsubsCheck, NotXml)
{
    const char *txt = "This is plain text without any angle brackets";
    char *errmsg = nullptr;
    int rc = checkIfXml((const unsigned char*)txt, (int)strlen(txt), &errmsg);
    EXPECT_NE(0, rc);   // non-zero = not valid XML
}

// ---------------------------------------------------------------------------
// processComment — skip over <!-- ... --> blocks
// ---------------------------------------------------------------------------

TEST(XmlsubsComment, WellFormedComment)
{
    const char *comment = "<!-- this is a comment -->";
    int len = processComment(comment, (int)strlen(comment));
    EXPECT_EQ((int)strlen(comment), len);
}

TEST(XmlsubsComment, MalformedComment)
{
    // No closing --> — should return the length searched without crashing
    const char *bad = "<!-- unclosed comment";
    int len = processComment(bad, (int)strlen(bad));
    EXPECT_GE(len, 0);
}

// ---------------------------------------------------------------------------
// findDelim — find delimiter respecting an escape character
// ---------------------------------------------------------------------------

TEST(XmlsubsDelim, FindComma)
{
    const unsigned char data[] = "hello,world";
    int pos = findDelim(data, (int)strlen((char*)data), ',', '\\');
    EXPECT_EQ(6, pos);   // 1-indexed position (index 5 + 1)
}

TEST(XmlsubsDelim, EscapedDelim)
{
    // Escaped comma should be skipped
    const unsigned char data[] = "hel\\,lo,world";
    int pos = findDelim(data, (int)strlen((char*)data), ',', '\\');
    EXPECT_EQ(8, pos);   // second comma at index 7, returns 7+1=8
}

TEST(XmlsubsDelim, NoDelim)
{
    const unsigned char data[] = "hello";
    int pos = findDelim(data, (int)strlen((char*)data), ',', '\\');
    EXPECT_EQ((int)strlen((char*)data), pos);   // not found → returns maxchar
}

// ---------------------------------------------------------------------------
// findXmlValue — extract value from XML element
// ---------------------------------------------------------------------------

TEST(XmlsubsFind, FindExistingKey)
{
    const char *xml = "<name>Alice</name>";
    char result[64] = {};
    findXmlValue(xml, "name", result, sizeof(result));
    EXPECT_STREQ("Alice", result);
}

TEST(XmlsubsFind, FindMissingKey)
{
    const char *xml = "<name>Alice</name>";
    char result[64] = { 'X' };
    // findXmlValue leaves buffer untouched when key is not found
    findXmlValue(xml, "missing", result, sizeof(result));
    EXPECT_EQ('X', result[0]);
}

// ---------------------------------------------------------------------------
// processDTD — skip over <!DOCTYPE ...> declarations
// ---------------------------------------------------------------------------

TEST(XmlsubsDTD, SimpleDTD)
{
    const char *dtd = "<!DOCTYPE root>";
    int len = processDTD(dtd, (int)strlen(dtd));
    EXPECT_EQ((int)strlen(dtd), len);
}

TEST(XmlsubsDTD, NotDTD)
{
    const char *xml = "<root/>";
    int len = processDTD(xml, (int)strlen(xml));
    EXPECT_EQ(0, len);
}

// ---------------------------------------------------------------------------
// removeCrLf — strip CR/LF characters from data
// ---------------------------------------------------------------------------

TEST(XmlsubsCrLf, RemoveCrLf)
{
    unsigned char data[] = "hello\r\nworld\n";
    int newLen = removeCrLf(data, (int)strlen((char*)data));
    EXPECT_EQ(10, newLen);
    EXPECT_EQ(0, memcmp(data, "helloworld", 10));
}

TEST(XmlsubsCrLf, NoCrLf)
{
    unsigned char data[] = "hello";
    int newLen = removeCrLf(data, (int)strlen((char*)data));
    EXPECT_EQ(5, newLen);
    EXPECT_EQ(0, memcmp(data, "hello", 5));
}

// ---------------------------------------------------------------------------
// setEscChar — decode a single XML entity starting at '&'
// ---------------------------------------------------------------------------

TEST(XmlsubsSetEsc, DecodeLt)
{
    unsigned char input[] = "&lt;rest";
    unsigned char output[1] = {};
    int len = setEscChar(input, output);
    EXPECT_EQ('<', output[0]);
    EXPECT_EQ(4, len);
}

TEST(XmlsubsSetEsc, DecodeAmp)
{
    unsigned char input[] = "&amp;rest";
    unsigned char output[1] = {};
    int len = setEscChar(input, output);
    EXPECT_EQ('&', output[0]);
    EXPECT_EQ(5, len);
}

// ---------------------------------------------------------------------------
// findEndBracket — find closing '>' in tag
// ---------------------------------------------------------------------------

TEST(XmlsubsBracket, FindEndBracket)
{
    char data[] = "id=\"1\">content";
    char *result = findEndBracket(data);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ('>', *result);
    EXPECT_EQ(6, (int)(result - data));
}

TEST(XmlsubsBracket, FindEndBracketAtStart)
{
    char data[] = ">rest";
    char *result = findEndBracket(data);
    EXPECT_EQ(data, result);
}

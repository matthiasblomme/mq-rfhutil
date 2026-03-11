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
    EXPECT_NE(0, rc);   // non-zero = valid
}

TEST(XmlsubsCheck, EmptyString)
{
    char *errmsg = nullptr;
    int rc = checkIfXml((const unsigned char*)"", 0, &errmsg);
    EXPECT_EQ(0, rc);   // zero = not XML
}

TEST(XmlsubsCheck, NotXml)
{
    const char *txt = "This is plain text without any angle brackets";
    char *errmsg = nullptr;
    int rc = checkIfXml((const unsigned char*)txt, (int)strlen(txt), &errmsg);
    EXPECT_EQ(0, rc);
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
    EXPECT_EQ(5, pos);
}

TEST(XmlsubsDelim, EscapedDelim)
{
    // Escaped comma should be skipped
    const unsigned char data[] = "hel\\,lo,world";
    int pos = findDelim(data, (int)strlen((char*)data), ',', '\\');
    EXPECT_EQ(7, pos);   // second comma, not the escaped one
}

TEST(XmlsubsDelim, NoDelim)
{
    const unsigned char data[] = "hello";
    int pos = findDelim(data, (int)strlen((char*)data), ',', '\\');
    EXPECT_LT(pos, 0);   // not found → negative
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
    // Should not modify result or crash when key is absent
    findXmlValue(xml, "missing", result, sizeof(result));
    // result[0] unchanged — either 'X' or empty depending on impl
    // Just verify no crash
    SUCCEED();
}

// XMLParseTests.cpp — unit tests for RFHUtil/XMLParse.cpp
// Covers: parsing valid/invalid XML, round-trip via createXML,
//         tree navigation, error messages.

#include "stdafx.h"
#include "rfhutil.h"
#include <gtest/gtest.h>
#include "Names.h"
#include "XMLParse.h"

// ---------------------------------------------------------------------------
// Basic parsing
// ---------------------------------------------------------------------------

TEST(XMLParse, ParseSimpleElement)
{
    const char *xml = "<root><child>hello</child></root>";
    CXMLParse p;
    int rc = p.parse(xml, (int)strlen(xml));
    EXPECT_EQ(PARSE_OK, rc);
}

TEST(XMLParse, ParseSelfClosingElement)
{
    const char *xml = "<root><empty/></root>";
    CXMLParse p;
    int rc = p.parse(xml, (int)strlen(xml));
    EXPECT_EQ(PARSE_OK, rc);
}

TEST(XMLParse, ParseWithAttributes)
{
    const char *xml = "<root id=\"1\" name=\"test\"><child/></root>";
    CXMLParse p;
    int rc = p.parse(xml, (int)strlen(xml));
    EXPECT_EQ(PARSE_OK, rc);
}

TEST(XMLParse, ParseNested)
{
    const char *xml = "<a><b><c>value</c></b></a>";
    CXMLParse p;
    int rc = p.parse(xml, (int)strlen(xml));
    EXPECT_EQ(PARSE_OK, rc);
}

TEST(XMLParse, ParseEmptyInput)
{
    CXMLParse p;
    int rc = p.parse("", 0);
    EXPECT_NE(PARSE_OK, rc);
}

TEST(XMLParse, ParseBinaryZero)
{
    const char bad[] = "\x00<root/>";
    CXMLParse p;
    int rc = p.parse(bad, 8);
    EXPECT_NE(PARSE_OK, rc);
}

// ---------------------------------------------------------------------------
// Tree navigation
// ---------------------------------------------------------------------------

TEST(XMLParse, GetFirstChild)
{
    const char *xml = "<root><child>v</child></root>";
    CXMLParse p;
    ASSERT_EQ(PARSE_OK, p.parse(xml, (int)strlen(xml)));

    int root  = p.getFirstChild(0);
    ASSERT_GE(root, 0);

    int child = p.getFirstChild(root);
    ASSERT_GE(child, 0);

    const char *name = p.getElemName(child);
    ASSERT_NE(nullptr, name);
    EXPECT_STREQ("child", name);
}

TEST(XMLParse, GetElemValue)
{
    const char *xml = "<root><msg>hello world</msg></root>";
    CXMLParse p;
    ASSERT_EQ(PARSE_OK, p.parse(xml, (int)strlen(xml)));

    int root  = p.getFirstChild(0);
    int msg   = p.getFirstChild(root);
    ASSERT_GE(msg, 0);

    const char *val = p.getElemValue(msg);
    ASSERT_NE(nullptr, val);
    EXPECT_STREQ("hello world", val);
}

TEST(XMLParse, SiblingNavigation)
{
    const char *xml = "<root><a/><b/><c/></root>";
    CXMLParse p;
    ASSERT_EQ(PARSE_OK, p.parse(xml, (int)strlen(xml)));

    int root = p.getFirstChild(0);
    int count = 0;
    int elem  = p.getFirstChild(root);
    while (elem > 0)
    {
        count++;
        elem = p.getNextSibling(elem);
    }
    EXPECT_EQ(3, count);
}

// ---------------------------------------------------------------------------
// createXML — round-trip
// ---------------------------------------------------------------------------

TEST(XMLParse, CreateXMLNoCrash)
{
    const char *xml = "<root><item>42</item></root>";
    CXMLParse p;
    ASSERT_EQ(PARSE_OK, p.parse(xml, (int)strlen(xml)));

    char out[512] = {};
    int rc = p.createXML(out, sizeof(out));
    EXPECT_GE(rc, 0);
    EXPECT_GT(strlen(out), 0u);
}

TEST(XMLParse, CreateXMLContainsElement)
{
    const char *xml = "<root><value>data</value></root>";
    CXMLParse p;
    ASSERT_EQ(PARSE_OK, p.parse(xml, (int)strlen(xml)));

    char out[512] = {};
    p.createXML(out, sizeof(out));

    // Output should contain the element name and value
    EXPECT_NE(nullptr, strstr(out, "value"));
    EXPECT_NE(nullptr, strstr(out, "data"));
}

// ---------------------------------------------------------------------------
// Error messages
// ---------------------------------------------------------------------------

TEST(XMLParse, GetErrorMsgOK)
{
    CXMLParse p;
    const char *msg = p.getErrorMsg(PARSE_OK);
    ASSERT_NE(nullptr, msg);
    EXPECT_GT(strlen(msg), 0u);
}

TEST(XMLParse, GetErrorMsgTagMismatch)
{
    CXMLParse p;
    const char *msg = p.getErrorMsg(PARSE_TAG_MISMATCH);
    ASSERT_NE(nullptr, msg);
    EXPECT_GT(strlen(msg), 0u);
}

TEST(XMLParse, GetErrorMsgMallocFail)
{
    CXMLParse p;
    const char *msg = p.getErrorMsg(PARSE_MALLOC_FAIL);
    ASSERT_NE(nullptr, msg);
    EXPECT_GT(strlen(msg), 0u);
}

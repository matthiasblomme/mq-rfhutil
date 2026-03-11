// JsonParseTests.cpp — unit tests for RFHUtil/JsonParse.cpp
// Covers: parsing flat and nested JSON, tree navigation, error handling.

#include "stdafx.h"
#include "rfhutil.h"
#include <gtest/gtest.h>
#include "Names.h"
#include "JsonParse.h"

// ---------------------------------------------------------------------------
// Basic parsing
// ---------------------------------------------------------------------------

TEST(JsonParse, ParseFlatObject)
{
    const char *json = "{\"name\":\"Alice\",\"age\":\"30\"}";
    CJsonParse p;
    int rc = p.parse(json, (int)strlen(json));
    EXPECT_EQ(JPARSE_OK, rc);
}

TEST(JsonParse, ParseEmpty)
{
    const char *json = "{}";
    CJsonParse p;
    int rc = p.parse(json, (int)strlen(json));
    EXPECT_EQ(JPARSE_OK, rc);
}

TEST(JsonParse, ParseInvalidStartChar)
{
    const char *bad = "not json at all";
    CJsonParse p;
    int rc = p.parse(bad, (int)strlen(bad));
    EXPECT_NE(JPARSE_OK, rc);
}

TEST(JsonParse, ParseNested)
{
    const char *json = "{\"person\":{\"name\":\"Bob\",\"city\":\"London\"}}";
    CJsonParse p;
    int rc = p.parse(json, (int)strlen(json));
    EXPECT_EQ(JPARSE_OK, rc);
}

TEST(JsonParse, ParseArray)
{
    const char *json = "{\"items\":[\"one\",\"two\",\"three\"]}";
    CJsonParse p;
    int rc = p.parse(json, (int)strlen(json));
    EXPECT_EQ(JPARSE_OK, rc);
}

// ---------------------------------------------------------------------------
// Tree navigation after successful parse
// ---------------------------------------------------------------------------

TEST(JsonParse, NavigateFirstChild)
{
    const char *json = "{\"key\":\"value\"}";
    CJsonParse p;
    ASSERT_EQ(JPARSE_OK, p.parse(json, (int)strlen(json)));

    int child = p.getFirstChild(0);
    EXPECT_GE(child, 0);
}

TEST(JsonParse, GetElemName)
{
    const char *json = "{\"greeting\":\"hello\"}";
    CJsonParse p;
    ASSERT_EQ(JPARSE_OK, p.parse(json, (int)strlen(json)));

    int child = p.getFirstChild(0);
    ASSERT_GE(child, 0);

    const char *name = p.getElemName(child);
    ASSERT_NE(nullptr, name);
    EXPECT_STREQ("greeting", name);
}

TEST(JsonParse, GetElemValue)
{
    const char *json = "{\"msg\":\"world\"}";
    CJsonParse p;
    ASSERT_EQ(JPARSE_OK, p.parse(json, (int)strlen(json)));

    int child = p.getFirstChild(0);
    ASSERT_GE(child, 0);

    const char *val = p.getElemValue(child);
    ASSERT_NE(nullptr, val);
    EXPECT_STREQ("world", val);
}

TEST(JsonParse, NoNextSiblingForSingleKey)
{
    const char *json = "{\"only\":\"one\"}";
    CJsonParse p;
    ASSERT_EQ(JPARSE_OK, p.parse(json, (int)strlen(json)));

    int child = p.getFirstChild(0);
    ASSERT_GE(child, 0);

    // Only one key — next sibling should be -1 or 0 (not found)
    int sib = p.getNextSibling(child);
    EXPECT_LT(sib, 1);
}

TEST(JsonParse, MultipleKeysNavigation)
{
    const char *json = "{\"a\":\"1\",\"b\":\"2\",\"c\":\"3\"}";
    CJsonParse p;
    ASSERT_EQ(JPARSE_OK, p.parse(json, (int)strlen(json)));

    // Count children
    int count = 0;
    int elem = p.getFirstChild(0);
    while (elem > 0)
    {
        count++;
        elem = p.getNextSibling(elem);
    }
    EXPECT_EQ(3, count);
}

// ---------------------------------------------------------------------------
// buildParsedArea — reconstruct JSON from parsed tree
// ---------------------------------------------------------------------------

TEST(JsonParse, BuildParsedAreaNoCrash)
{
    const char *json = "{\"x\":\"42\"}";
    CJsonParse p;
    ASSERT_EQ(JPARSE_OK, p.parse(json, (int)strlen(json)));

    char out[256] = {};
    p.buildParsedArea(out, sizeof(out), TRUE);
    EXPECT_GT(strlen(out), 0u);
}

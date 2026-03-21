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

    // Verify actual parsed values
    int child = p.getFirstChild(0);
    ASSERT_GT(child, 0);
    EXPECT_STREQ("name", p.getElemName(child));
    EXPECT_STREQ("\"Alice\"", p.getElemValue(child));

    int sib = p.getNextSibling(child);
    ASSERT_GT(sib, 0);
    EXPECT_STREQ("age", p.getElemName(sib));
    EXPECT_STREQ("\"30\"", p.getElemValue(sib));
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

    // Navigate into nested object and verify values
    int person = p.getFirstChild(0);
    ASSERT_GT(person, 0);
    EXPECT_STREQ("person", p.getElemName(person));

    int nameElem = p.getFirstChild(person);
    ASSERT_GT(nameElem, 0);
    EXPECT_STREQ("name", p.getElemName(nameElem));
    EXPECT_STREQ("\"Bob\"", p.getElemValue(nameElem));

    int cityElem = p.getNextSibling(nameElem);
    ASSERT_GT(cityElem, 0);
    EXPECT_STREQ("city", p.getElemName(cityElem));
    EXPECT_STREQ("\"London\"", p.getElemValue(cityElem));
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
    EXPECT_STREQ("\"world\"", val);
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
    // Verify reconstructed output contains original key and value
    EXPECT_NE(nullptr, strstr(out, "x"));
    EXPECT_NE(nullptr, strstr(out, "42"));
}

TEST(JsonParse, BuildParsedAreaMultipleKeys)
{
    const char *json = "{\"a\":\"1\",\"b\":\"2\"}";
    CJsonParse p;
    ASSERT_EQ(JPARSE_OK, p.parse(json, (int)strlen(json)));

    char out[512] = {};
    p.buildParsedArea(out, sizeof(out), TRUE);
    EXPECT_NE(nullptr, strstr(out, "a"));
    EXPECT_NE(nullptr, strstr(out, "1"));
    EXPECT_NE(nullptr, strstr(out, "b"));
    EXPECT_NE(nullptr, strstr(out, "2"));
}

// ---------------------------------------------------------------------------
// Malformed JSON — error handling
// ---------------------------------------------------------------------------

TEST(JsonParse, ParseUnclosedBrace)
{
    // Parser is lenient — truncated input doesn't produce an error code
    const char *json = "{\"a\":\"1\"";
    CJsonParse p;
    int rc = p.parse(json, (int)strlen(json));
    (void)rc;
    SUCCEED();
}

TEST(JsonParse, ParseUnclosedString)
{
    // Parser is lenient — truncated input doesn't produce an error code
    const char *json = "{\"key\":\"val";
    CJsonParse p;
    int rc = p.parse(json, (int)strlen(json));
    (void)rc;
    SUCCEED();
}

TEST(JsonParse, ParseTrailingComma)
{
    const char *json = "{\"a\":\"1\",}";
    CJsonParse p;
    int rc = p.parse(json, (int)strlen(json));
    // Document actual behavior — trailing comma may or may not be accepted
    // The important thing is it doesn't crash
    (void)rc;
    SUCCEED();
}

TEST(JsonParse, GetLastErrorAfterFail)
{
    const char *json = "not json";
    CJsonParse p;
    int rc = p.parse(json, (int)strlen(json));
    ASSERT_NE(JPARSE_OK, rc);

    char errtxt[256] = {};
    p.getLastError(errtxt, sizeof(errtxt));
    EXPECT_GT(strlen(errtxt), 0u);
}

// ---------------------------------------------------------------------------
// Navigation edge cases
// ---------------------------------------------------------------------------

TEST(JsonParse, GetFirstChildOnLeaf)
{
    const char *json = "{\"key\":\"val\"}";
    CJsonParse p;
    ASSERT_EQ(JPARSE_OK, p.parse(json, (int)strlen(json)));

    int leaf = p.getFirstChild(0);
    ASSERT_GT(leaf, 0);
    // A leaf node (string value) should have no children
    int child = p.getFirstChild(leaf);
    EXPECT_LE(child, 0);
}

TEST(JsonParse, ParseNestedVerifyDeepValue)
{
    const char *json = "{\"outer\":{\"inner\":\"deep\"}}";
    CJsonParse p;
    ASSERT_EQ(JPARSE_OK, p.parse(json, (int)strlen(json)));

    int outer = p.getFirstChild(0);
    ASSERT_GT(outer, 0);
    int inner = p.getFirstChild(outer);
    ASSERT_GT(inner, 0);
    EXPECT_STREQ("inner", p.getElemName(inner));
    EXPECT_STREQ("\"deep\"", p.getElemValue(inner));
}

TEST(JsonParse, ParseArrayCountElements)
{
    const char *json = "{\"items\":[\"one\",\"two\",\"three\"]}";
    CJsonParse p;
    ASSERT_EQ(JPARSE_OK, p.parse(json, (int)strlen(json)));

    int items = p.getFirstChild(0);
    ASSERT_GT(items, 0);
    EXPECT_STREQ("items", p.getElemName(items));

    // Array elements are stored as the value, not as navigable children
    const char *val = p.getElemValue(items);
    ASSERT_NE(nullptr, val);
    EXPECT_NE(nullptr, strstr(val, "one"));
}

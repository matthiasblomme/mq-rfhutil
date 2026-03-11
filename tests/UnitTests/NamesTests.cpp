// NamesTests.cpp — unit tests for RFHUtil/Names.cpp
// Covers: insert, find, getNameAddr, iteration, reallocation, statistics.

#include "stdafx.h"
#include "rfhutil.h"
#include <gtest/gtest.h>
#include "Names.h"

// ---------------------------------------------------------------------------
// Insert
// ---------------------------------------------------------------------------

TEST(NamesInsert, SingleStringReturnsNonZeroHandle)
{
    Names names;
    int handle = names.insertName("hello");
    EXPECT_GT(handle, 0);
}

TEST(NamesInsert, DifferentStringsGetDifferentHandles)
{
    Names names;
    int h1 = names.insertName("alpha");
    int h2 = names.insertName("beta");
    EXPECT_NE(h1, h2);
    EXPECT_GT(h1, 0);
    EXPECT_GT(h2, 0);
}

TEST(NamesInsert, DuplicateReturnsSameHandle)
{
    Names names;
    int h1 = names.insertName("hello");
    int h2 = names.insertName("hello");
    EXPECT_EQ(h1, h2);
}

TEST(NamesInsert, DuplicateDoesNotIncrementInsertCount)
{
    Names names;
    names.insertName("hello");
    EXPECT_EQ(1, names.getInsertCount());
    names.insertName("hello");
    EXPECT_EQ(1, names.getInsertCount());
}

TEST(NamesInsert, EmptyStringReturnsZero)
{
    Names names;
    EXPECT_EQ(0, names.insertName(""));
}

TEST(NamesInsert, WithExplicitLength)
{
    Names names;
    int handle = names.insertName("hello", 5);
    EXPECT_GT(handle, 0);
    // The full 5-char key is distinct from a shorter prefix
    EXPECT_EQ(0, names.findName("hell", 4));
}

// ---------------------------------------------------------------------------
// Find
// ---------------------------------------------------------------------------

TEST(NamesFind, ExistingStringFound)
{
    Names names;
    int inserted = names.insertName("world");
    int found    = names.findName("world");
    EXPECT_EQ(inserted, found);
}

TEST(NamesFind, NonExistentReturnsZero)
{
    Names names;
    EXPECT_EQ(0, names.findName("nothere"));
}

TEST(NamesFind, WithExplicitLengthDistinguishesPrefixes)
{
    Names names;
    names.insertName("hello", 5);
    EXPECT_GT(names.findName("hello", 5), 0);
    EXPECT_EQ(0,  names.findName("hell",  4));  // shorter prefix not inserted
}

TEST(NamesFind, CaseSensitive)
{
    Names names;
    names.insertName("Hello");
    EXPECT_EQ(0, names.findName("hello"));   // lowercase not found
    EXPECT_EQ(0, names.findName("HELLO"));   // uppercase not found
    EXPECT_GT(names.findName("Hello"), 0);   // original casing found
}

TEST(NamesFind, MultipleDistinctStrings)
{
    Names names;
    int h1 = names.insertName("foo");
    int h2 = names.insertName("bar");
    int h3 = names.insertName("baz");
    EXPECT_EQ(h1, names.findName("foo"));
    EXPECT_EQ(h2, names.findName("bar"));
    EXPECT_EQ(h3, names.findName("baz"));
}

// ---------------------------------------------------------------------------
// getNameAddr
// ---------------------------------------------------------------------------

TEST(NamesAddress, ReturnsCorrectString)
{
    Names names;
    int handle = names.insertName("rfhutil");
    char *ptr  = names.getNameAddr(handle);
    EXPECT_STREQ("rfhutil", ptr);
}

TEST(NamesAddress, MultipleEntriesEachCorrect)
{
    Names names;
    int h1 = names.insertName("alpha");
    int h2 = names.insertName("beta");
    int h3 = names.insertName("gamma");
    EXPECT_STREQ("alpha", names.getNameAddr(h1));
    EXPECT_STREQ("beta",  names.getNameAddr(h2));
    EXPECT_STREQ("gamma", names.getNameAddr(h3));
}

// ---------------------------------------------------------------------------
// Iteration (getFirstEntry / getNextEntry)
// ---------------------------------------------------------------------------

TEST(NamesIteration, EmptyTableFirstEntryIsZero)
{
    Names names;
    EXPECT_EQ(0, names.getFirstEntry());
}

TEST(NamesIteration, SingleEntryMatchesInsertedHandle)
{
    Names names;
    int handle = names.insertName("one");
    EXPECT_EQ(handle, names.getFirstEntry());
}

TEST(NamesIteration, NextEntryOnEmptyIsZero)
{
    Names names;
    EXPECT_EQ(0, names.getNextEntry(0));
}

TEST(NamesIteration, IterateCountMatchesInsertCount)
{
    Names names;
    names.insertName("alpha");
    names.insertName("beta");
    names.insertName("gamma");

    int count = 0;
    int ofs   = names.getFirstEntry();
    while (ofs > 0)
    {
        count++;
        ofs = names.getNextEntry(ofs);
    }
    EXPECT_EQ(3, count);
}

TEST(NamesIteration, IteratedStringsMatchInserted)
{
    Names names;
    names.insertName("first");
    names.insertName("second");
    names.insertName("third");

    // Collect strings via iteration
    bool foundFirst  = false;
    bool foundSecond = false;
    bool foundThird  = false;

    int ofs = names.getFirstEntry();
    while (ofs > 0)
    {
        const char *s = names.getNameAddr(ofs);
        if (strcmp(s, "first")  == 0) foundFirst  = true;
        if (strcmp(s, "second") == 0) foundSecond = true;
        if (strcmp(s, "third")  == 0) foundThird  = true;
        ofs = names.getNextEntry(ofs);
    }

    EXPECT_TRUE(foundFirst);
    EXPECT_TRUE(foundSecond);
    EXPECT_TRUE(foundThird);
}

// ---------------------------------------------------------------------------
// Reallocation
// ---------------------------------------------------------------------------

TEST(NamesReallocation, ManyInsertsAllSucceed)
{
    Names names(64);   // small initial table forces reallocations
    char  buf[32];
    const int COUNT = 50;
    for (int i = 0; i < COUNT; i++)
    {
        sprintf(buf, "name%d", i);
        EXPECT_GT(names.insertName(buf), 0);
    }
    EXPECT_EQ(COUNT, names.getInsertCount());
}

TEST(NamesReallocation, OldEntriesStillValidAfterRealloc)
{
    Names names(64);   // small initial table
    char  expected[20][32];
    int   handles[20];
    char  buf[32];

    for (int i = 0; i < 20; i++)
    {
        sprintf(buf, "entry%d", i);
        strcpy(expected[i], buf);
        handles[i] = names.insertName(buf);
    }

    // All entries must still be readable after potential reallocations
    for (int i = 0; i < 20; i++)
        EXPECT_STREQ(expected[i], names.getNameAddr(handles[i]));
}

TEST(NamesReallocation, ReallocCountIncrements)
{
    Names names(64);   // small initial table
    int   before = names.getReallocCount();
    char  buf[32];
    for (int i = 0; i < 20; i++)
    {
        sprintf(buf, "longname%02d", i);
        names.insertName(buf);
    }
    EXPECT_GT(names.getReallocCount(), before);
}

// ---------------------------------------------------------------------------
// Statistics
// ---------------------------------------------------------------------------

TEST(NamesStatistics, InitialInsertCountIsZero)
{
    Names names;
    EXPECT_EQ(0, names.getInsertCount());
}

TEST(NamesStatistics, InsertCountIncrementsPerUniqueString)
{
    Names names;
    names.insertName("a");
    EXPECT_EQ(1, names.getInsertCount());
    names.insertName("b");
    EXPECT_EQ(2, names.getInsertCount());
    names.insertName("c");
    EXPECT_EQ(3, names.getInsertCount());
}

TEST(NamesStatistics, DefaultTableSizeIsPositive)
{
    Names names;
    EXPECT_GT(names.getNameTableSize(), 0);
}

TEST(NamesStatistics, CustomInitSizeIsHonoured)
{
    const int SIZE = 1024;
    Names names(SIZE);
    EXPECT_EQ(SIZE, names.getNameTableSize());
}

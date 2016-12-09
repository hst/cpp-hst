/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "test-cases.h"
#include "test-harness.cc.in"

TEST_CASE_GROUP("test harness group 1");

TEST_CASE("can run a test case")
{
}

TEST_CASE("can compare things for equality")
{
    check_eq(0, 0);
    check_ne(0, 1);
    check_eq("a", "a");
    check_ne("a", "b");
}

#if 0
// This is what a failure looks like; we can't actually run it because that will
// be treated as a real test failure!  Uncomment this when you want to manually
// verify that failures work.

TEST_CASE("can fail a test case")
{
    fail() << "whoops";
}

TEST_CASE("can fail an equality test")
{
    check_eq(0, 1);
}
#endif

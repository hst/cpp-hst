/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "test-cases.h"
#include "test-harness.cc.in"

#include "hst.h"

using hst::Result;
using std::string;

static Result<int, string>
good(int value)
{
    return value;
}

static Result<int, string>
bad()
{
    return string("Returning an error");
}

TEST_CASE_GROUP("results");

TEST_CASE("can produce a successful result")
{
    Result<int, string> result = good(12);
    check_eq(result.valid(), true);
    check_eq(result.get(), 12);
}

TEST_CASE("can produce a failed result")
{
    Result<int, string> result = bad();
    check_eq(result.valid(), false);
    check_eq(result.get_error(), "Returning an error");
}

/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "test-cases.h"
#include "test-harness.cc.in"

#include "hst/event.h"

using hst::Event;

TEST_CASE_GROUP("events");

TEST_CASE("can create events")
{
    Event a("a");
    Event b("b");
    Event tau("τ");
    check_ne(Event::none(), a);
    check_ne(Event::none(), b);
    check_ne(Event::none(), tau);
}

TEST_CASE("events are interned")
{
    Event a1("a");
    Event a2("a");
    check_eq(a1, a2);
}

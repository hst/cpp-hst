/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <initializer_list>
#include <utility>

#include "test-cases.h"
#include "test-harness.cc.in"

#include "hst.h"

using hst::Event;
using hst::LTS;
using hst::Process;
using hst::ProcessSet;

static std::pair<Event, ProcessSet>
edge(Event event, std::initializer_list<Process> processes)
{
    return std::make_pair(event, ProcessSet(processes));
}

TEST_CASE_GROUP("LTS");

TEST_CASE("can create an empty LTS")
{
    LTS lts;
}

TEST_CASE("can add nodes to an LTS")
{
    LTS lts;
    Process p1 = lts.add_process();
    Process p2 = lts.add_process();
    Process p3 = lts.add_process();
    check_ne(p1, p2);
    check_ne(p1, p3);
}

TEST_CASE("can add edges to an LTS")
{
    LTS lts;
    Process stop = lts.add_process();
    Process p1 = lts.add_process();
    lts.add_edge(p1, "a", stop);
    check_eq(lts.transitions(stop), LTS::TransitionsMap{});
    check_eq(lts.transitions(p1), LTS::TransitionsMap{edge("a", {stop})});
    check_eq(lts.afters(p1, "a"), ProcessSet{stop});
}

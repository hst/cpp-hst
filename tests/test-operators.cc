/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <initializer_list>
#include <string>

#include "test-cases.h"
#include "test-harness.cc.in"

#include "hst/event.h"
#include "hst/prefix.h"
#include "hst/process.h"
#include "hst/stop.h"

using hst::Event;
using hst::Prefix;
using hst::Process;
using hst::Stop;

namespace {

Event::Set
events_from_names(std::initializer_list<const std::string> names)
{
    Event::Set set;
    for (const auto& name : names) {
        set.insert(Event(name));
    }
    return set;
}

void
check_initials(const std::shared_ptr<Process>& process,
               std::initializer_list<const std::string> expected)
{
    Event::Set actual;
    process->initials(&actual);
    check_eq(actual, events_from_names(expected));
}

void
check_afters(const std::shared_ptr<Process>& process,
             const std::string& initial,
             std::initializer_list<std::shared_ptr<Process>> expected)
{
    Process::Set actual;
    process->afters(Event(initial), &actual);
    check_eq(actual, Process::Set(expected));
}

}  // namespace

TEST_CASE_GROUP("process comparisons");

TEST_CASE("can compare individual processes")
{
    auto p1 = Prefix::create(Event("a"), Stop::create());
    auto p2 = Prefix::create(Event("a"), Stop::create());
    check_eq(*p1, *p1);
    check_eq(*p1, *p2);
}

TEST_CASE("can compare sets of processes")
{
    auto p1 = Prefix::create(Event("a"), Stop::create());
    auto p2 = Prefix::create(Event("a"), Stop::create());
    Process::Set set1{p1};
    Process::Set set2{p2};
    check_eq(set1, set1);
    check_eq(set1, set2);
}

TEST_CASE_GROUP("prefix");

TEST_CASE("a → STOP")
{
    auto p = Prefix::create(Event("a"), Stop::create());
    check_initials(p, {"a"});
    check_afters(p, "a", {Stop::create()});
    check_afters(p, "τ", {});
}

TEST_CASE_GROUP("STOP");

TEST_CASE("STOP")
{
    auto stop = Stop::create();
    check_initials(stop, {});
    check_afters(stop, "a", {});
    check_afters(stop, "τ", {});
}

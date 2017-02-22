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
check_initials(Process* process,
               std::initializer_list<const std::string> expected)
{
    Event::Set actual;
    process->initials(&actual);
    check_eq(actual, events_from_names(expected));
}

void
check_afters(Process* process, const std::string& initial,
             std::initializer_list<Process*> expected)
{
    Process::Set actual;
    process->afters(Event(initial), &actual);
    check_eq(actual, Process::Set(expected));
}

}  // namespace

TEST_CASE_GROUP("prefix");

TEST_CASE("a → STOP")
{
    Prefix p(Event("a"), Stop::get());
    check_initials(&p, {"a"});
    check_afters(&p, "a", {Stop::get()});
    check_afters(&p, "τ", {});
}

TEST_CASE_GROUP("STOP");

TEST_CASE("STOP")
{
    Stop* stop = Stop::get();
    check_initials(stop, {});
    check_afters(stop, "a", {});
    check_afters(stop, "τ", {});
}

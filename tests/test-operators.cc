/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <initializer_list>
#include <sstream>
#include <string>

#include "test-cases.h"
#include "test-harness.cc.in"

#include "hst/csp0.h"
#include "hst/event.h"
#include "hst/process.h"

using hst::Event;
using hst::ParseError;
using hst::Process;

// The test cases in this file verify that we've implemented each of the CSP
// operators correctly: specifically, that they have the right "initials" and
// "afters" sets, as defined by CSP's operational semantics.
//
// We've provided some helper functions that make these test cases easier to
// write.  In particular, you can assume that the CSP₀ parser works as expected;
// that will have been checked in test-csp0.c.

namespace {

std::shared_ptr<Process>
require_csp0(const std::string& csp0)
{
    ParseError error;
    std::shared_ptr<Process> parsed = hst::load_csp0_string(csp0, &error);
    if (!parsed) {
        fail() << "Could not parse " << csp0 << ": " << error << abort_test();
    }
    return parsed;
}

Process::Set
require_csp0_set(std::initializer_list<const std::string> csp0s)
{
    Process::Set set;
    for (const auto& csp0 : csp0s) {
        set.insert(require_csp0(csp0));
    }
    return set;
}

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
check_name(const std::string& csp0, const std::string& expected)
{
    std::shared_ptr<Process> process = require_csp0(csp0);
    std::stringstream actual;
    actual << *process;
    check_eq(actual.str(), expected);
}

void
check_initials(const std::string& csp0,
               std::initializer_list<const std::string> expected)
{
    std::shared_ptr<Process> process = require_csp0(csp0);
    Event::Set actual;
    process->initials(&actual);
    check_eq(actual, events_from_names(expected));
}

void
check_afters(const std::string& csp0, const std::string& initial,
             std::initializer_list<const std::string> expected)
{
    std::shared_ptr<Process> process = require_csp0(csp0);
    Process::Set actual;
    process->afters(Event(initial), &actual);
    check_eq(actual, require_csp0_set(expected));
}

}  // namespace

TEST_CASE_GROUP("process comparisons");

TEST_CASE("can compare individual processes")
{
    auto p1 = require_csp0("a → STOP");
    auto p2 = require_csp0("a → STOP");
    check_eq(*p1, *p1);
    check_eq(*p1, *p2);
}

TEST_CASE("can compare sets of processes")
{
    auto p1 = require_csp0("a → STOP");
    auto p2 = require_csp0("a → STOP");
    Process::Set set1{p1};
    Process::Set set2{p2};
    check_eq(set1, set1);
    check_eq(set1, set2);
}

TEST_CASE_GROUP("external choice");

TEST_CASE("STOP □ STOP")
{
    auto p = "STOP □ STOP";
    check_name(p, "□ {STOP}");
    check_initials(p, {});
    check_afters(p, "a", {});
}

TEST_CASE("(a → STOP) □ (b → STOP ⊓ c → STOP)")
{
    auto p = "(a → STOP) □ (b → STOP ⊓ c → STOP)";
    check_name(p, "a → STOP □ (b → STOP ⊓ c → STOP)");
    check_initials(p, {"a", "τ"});
    check_afters(p, "a", {"STOP"});
    check_afters(p, "b", {});
    check_afters(p, "τ", {"a → STOP □ b → STOP", "a → STOP □ c → STOP"});
}

TEST_CASE("(a → STOP) □ (b → STOP)")
{
    auto p = "(a → STOP) □ (b → STOP)";
    check_name(p, "a → STOP □ b → STOP");
    check_initials(p, {"a", "b"});
    check_afters(p, "a", {"STOP"});
    check_afters(p, "b", {"STOP"});
    check_afters(p, "τ", {});
}

TEST_CASE("□ {a → STOP, b → STOP, c → STOP}")
{
    auto p = "□ {a → STOP, b → STOP, c → STOP}";
    check_name(p, "□ {a → STOP, b → STOP, c → STOP}");
    check_initials(p, {"a", "b", "c"});
    check_afters(p, "a", {"STOP"});
    check_afters(p, "b", {"STOP"});
    check_afters(p, "c", {"STOP"});
    check_afters(p, "τ", {});
}

TEST_CASE_GROUP("internal choice");

TEST_CASE("STOP ⊓ STOP")
{
    auto p = "STOP ⊓ STOP";
    check_name(p, "⊓ {STOP}");
    check_initials(p, {"τ"});
    check_afters(p, "τ", {"STOP"});
    check_afters(p, "a", {});
}

TEST_CASE("(a → STOP) ⊓ (b → STOP)")
{
    auto p = "(a → STOP) ⊓ (b → STOP)";
    check_name(p, "a → STOP ⊓ b → STOP");
    check_initials(p, {"τ"});
    check_afters(p, "τ", {"a → STOP", "b → STOP"});
    check_afters(p, "a", {});
}

TEST_CASE("⊓ {a → STOP, b → STOP, c → STOP}")
{
    auto p = "⊓ {a → STOP, b → STOP, c → STOP}";
    check_name(p, "⊓ {a → STOP, b → STOP, c → STOP}");
    check_initials(p, {"τ"});
    check_afters(p, "τ", {"a → STOP", "b → STOP", "c → STOP"});
    check_afters(p, "a", {});
}

TEST_CASE_GROUP("prefix");

TEST_CASE("a → STOP")
{
    auto p = "a → STOP";
    check_name(p, "a → STOP");
    check_initials(p, {"a"});
    check_afters(p, "a", {"STOP"});
    check_afters(p, "τ", {});
}

TEST_CASE("a → b → STOP")
{
    auto p = "a → b → STOP";
    check_name(p, "a → b → STOP");
    check_initials(p, {"a"});
    check_afters(p, "a", {"b → STOP"});
    check_afters(p, "τ", {});
}

TEST_CASE_GROUP("SKIP");

TEST_CASE("SKIP")
{
    auto skip = "SKIP";
    check_name(skip, "SKIP");
    check_initials(skip, {"✔"});
    check_afters(skip, "a", {});
    check_afters(skip, "τ", {});
    check_afters(skip, "✔", {"STOP"});
}

TEST_CASE_GROUP("STOP");

TEST_CASE("STOP")
{
    auto stop = "STOP";
    check_name(stop, "STOP");
    check_initials(stop, {});
    check_afters(stop, "a", {});
    check_afters(stop, "τ", {});
}

TEST_CASE_GROUP("sequential composition");

TEST_CASE("SKIP ; STOP")
{
    auto p = "SKIP ; STOP";
    check_name(p, "SKIP ; STOP");
    check_initials(p, {"τ"});
    check_afters(p, "a", {});
    check_afters(p, "b", {});
    check_afters(p, "τ", {"STOP"});
    check_afters(p, "✔", {});
}

TEST_CASE("a → SKIP ; STOP")
{
    auto p = "a → SKIP ; STOP";
    check_name(p, "a → SKIP ; STOP");
    check_initials(p, {"a"});
    check_afters(p, "a", {"SKIP ; STOP"});
    check_afters(p, "b", {});
    check_afters(p, "τ", {});
    check_afters(p, "✔", {});
}

TEST_CASE("(a → b → STOP □ SKIP) ; STOP")
{
    auto p = "(a → b → STOP □ SKIP) ; STOP";
    check_name(p, "(SKIP □ a → b → STOP) ; STOP");
    check_initials(p, {"a", "τ"});
    check_afters(p, "a", {"b → STOP ; STOP"});
    check_afters(p, "b", {});
    check_afters(p, "τ", {"STOP"});
    check_afters(p, "✔", {});
}

TEST_CASE("(a → b → STOP ⊓ SKIP) ; STOP")
{
    auto p = "(a → b → STOP ⊓ SKIP) ; STOP";
    check_name(p, "(SKIP ⊓ a → b → STOP) ; STOP");
    check_initials(p, {"τ"});
    check_afters(p, "a", {});
    check_afters(p, "b", {});
    check_afters(p, "τ", {"a → b → STOP ; STOP", "SKIP ; STOP"});
    check_afters(p, "✔", {});
}

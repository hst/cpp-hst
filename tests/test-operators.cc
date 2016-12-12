/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <initializer_list>
#include <set>
#include <string>

#include "test-cases.h"
#include "test-harness.cc.in"

#include "hst.h"

using hst::Alphabet;
using hst::Event;
using hst::LTS;
using hst::Process;
using hst::ProcessSet;

static Alphabet
alphabet(std::initializer_list<std::string> names_init)
{
    std::set<std::string> names(names_init);
    Alphabet events;
    for (const auto& name : names) {
        events.insert(Event(name));
    }
    return events;
}

static Alphabet
initials(const LTS& lts, Process process)
{
    Alphabet initials;
    for (const auto& transition : lts.transitions(process)) {
        initials.insert(transition.first);
    }
    return initials;
}

static void
check_initials(const LTS& lts, Process root,
               std::initializer_list<std::string> expected)
{
    check_eq(initials(lts, root), alphabet(expected));
}

static void
check_afters(const LTS& lts, Process root, const char* initial_name,
             std::initializer_list<Process> expected_init)
{
    Event initial(initial_name);
    ProcessSet expected(expected_init);
    check_eq(lts.afters(root, initial), expected);
}

TEST_CASE_GROUP("external choice");

TEST_CASE("STOP □ STOP")
{
    LTS lts;
    Process root = hst::external_choice(&lts, lts.stop, lts.stop);
    check_initials(lts, root, {});
    check_afters(lts, root, "a", {});
}

TEST_CASE("(a → STOP) □ (b → STOP)")
{
    LTS lts;
    Process p1 = hst::prefix(&lts, "a", lts.stop);
    Process p2 = hst::prefix(&lts, "b", lts.stop);
    Process root = hst::external_choice(&lts, p1, p2);
    check_initials(lts, root, {"a", "b"});
    check_afters(lts, root, "a", {lts.stop});
    check_afters(lts, root, "b", {lts.stop});
    check_afters(lts, root, "τ", {});
}

TEST_CASE("□ {a → STOP, b → STOP, c → STOP}")
{
    LTS lts;
    Process p1 = hst::prefix(&lts, "a", lts.stop);
    Process p2 = hst::prefix(&lts, "b", lts.stop);
    Process p3 = hst::prefix(&lts, "c", lts.stop);
    Process root = hst::external_choice(&lts, ProcessSet{p1, p2, p3});
    check_initials(lts, root, {"a", "b", "c"});
    check_afters(lts, root, "a", {lts.stop});
    check_afters(lts, root, "b", {lts.stop});
    check_afters(lts, root, "c", {lts.stop});
    check_afters(lts, root, "τ", {});
}

TEST_CASE_GROUP("prefix");

TEST_CASE("a → STOP")
{
    LTS lts;
    Process root = hst::prefix(&lts, "a", lts.stop);
    check_initials(lts, root, {"a"});
    check_afters(lts, root, "a", {lts.stop});
    check_afters(lts, root, "b", {});
}

TEST_CASE("a → b → STOP")
{
    LTS lts;
    Process p1 = hst::prefix(&lts, "b", lts.stop);
    Process root = hst::prefix(&lts, "a", p1);
    check_initials(lts, root, {"a"});
    check_afters(lts, root, "a", {p1});
    check_afters(lts, root, "b", {});
}

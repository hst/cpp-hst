/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <algorithm>
#include <assert.h>
#include <initializer_list>
#include <sstream>
#include <string>
#include <vector>

#include "test-cases.h"
#include "test-harness.cc.in"

#include "hst/csp0.h"
#include "hst/environment.h"
#include "hst/event.h"
#include "hst/process.h"
#include "hst/semantic-models.h"

using hst::Environment;
using hst::Event;
using hst::NormalizedProcess;
using hst::ParseError;
using hst::Process;
using hst::Trace;
using hst::Traces;

// The test cases in this file verify that we've implemented each of the CSP
// operators correctly: specifically, that they have the right "initials" and
// "afters" sets, as defined by CSP's operational semantics.
//
// We've provided some helper functions that make these test cases easier to
// write.  In particular, you can assume that the CSP₀ parser works as expected;
// that will have been checked in test-csp0.c.

namespace {

const Process*
require_csp0(Environment* env, const std::string& csp0)
{
    ParseError error;
    const Process* parsed = hst::load_csp0_string(env, csp0, &error);
    if (!parsed) {
        fail() << "Could not parse " << csp0 << ": " << error << abort_test();
    }
    return parsed;
}

Process::Set
require_csp0_set(Environment* env,
                 std::initializer_list<const std::string> csp0s)
{
    Process::Set set;
    for (const auto& csp0 : csp0s) {
        set.insert(require_csp0(env, csp0));
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

Trace
require_trace(std::initializer_list<const std::string> names)
{
    std::vector<Event> events;
    for (const auto& name : names) {
        events.emplace_back(name);
    }
    return Trace(std::move(events));
}

void
check_name(const std::string& csp0, const std::string& expected)
{
    Environment env;
    const Process* process = require_csp0(&env, csp0);
    std::stringstream actual;
    actual << *process;
    check_eq(actual.str(), expected);
}

void
check_subprocesses(const std::string& csp0,
                   std::initializer_list<const std::string> expected)
{
    Environment env;
    const Process* process = require_csp0(&env, csp0);
    Process::Set actual;
    process->subprocesses(&actual);
    check_eq(actual, require_csp0_set(&env, expected));
}

void
check_initials(const std::string& csp0,
               std::initializer_list<const std::string> expected)
{
    Environment env;
    const Process* process = require_csp0(&env, csp0);
    Event::Set actual;
    process->initials(&actual);
    check_eq(actual, events_from_names(expected));
}

void
check_afters(const std::string& csp0, const std::string& initial,
             std::initializer_list<const std::string> expected)
{
    Environment env;
    const Process* process = require_csp0(&env, csp0);
    Process::Set actual;
    process->afters(Event(initial), &actual);
    check_eq(actual, require_csp0_set(&env, expected));
}

// Verify all of the subprocesses that are reachable from `process`.
void
check_reachable(const std::string& csp0,
                std::initializer_list<const std::string> expected)
{
    Environment env;
    const Process* process = require_csp0(&env, csp0);
    Process::Set actual;
    process->bfs([&actual](const Process& process) {
        actual.insert(&process);
        return true;
    });
    check_eq(actual, require_csp0_set(&env, expected));
}

// Verify the τ-closure of `process`.
void
check_tau_closure(const std::string& csp0,
                  std::initializer_list<const std::string> expected)
{
    Environment env;
    const Process* process = require_csp0(&env, csp0);
    Process::Set actual{process};
    actual.tau_close();
    check_eq(actual, require_csp0_set(&env, expected));
}

void
check_traces_behavior(const std::string& csp0,
                      std::initializer_list<const std::string> expected)
{
    Environment env;
    const Process* process = require_csp0(&env, csp0);
    Traces::Behavior actual = Traces::get_process_behavior(*process);
    check_eq(actual.events(), events_from_names(expected));
}

// Verify that the given CSP₀ process has a particular set of maximal traces.
void
check_maximal_traces(
        const std::string& csp0,
        std::initializer_list<std::initializer_list<const std::string>>
                expected)
{
    Environment env;
    const Process* process = require_csp0(&env, csp0);
    std::vector<Trace> traces;
    for (const auto& trace : expected) {
        traces.emplace_back(require_trace(trace));
    }

    find_maximal_finite_traces(&env, process, [&traces](const Trace& trace) {
        auto it = std::find(traces.begin(), traces.end(), trace);
        if (it == traces.end()) {
            fail() << "Unexpected maximal trace " << trace << abort_test();
        }
        traces.erase(it);
    });

    if (!traces.empty()) {
        fail() << "Expected to find maximal trace " << traces[0]
               << abort_test();
    }
}

// Verify the set of non-normalized processes that a normalized process expands
// to.
void
check_expansion(const std::string& csp0,
                std::initializer_list<const std::string> expected)
{
    Environment env;
    const Process* process = require_csp0(&env, csp0);
    const NormalizedProcess* normalized =
            dynamic_cast<const NormalizedProcess*>(process);
    assert(normalized);
    Process::Set actual;
    normalized->expand(
            [&actual](const Process& process) { actual.insert(&process); });
    check_eq(actual, require_csp0_set(&env, expected));
}

}  // namespace

TEST_CASE_GROUP("process comparisons");

TEST_CASE("can compare individual processes")
{
    Environment env;
    auto p1 = require_csp0(&env, "a → STOP");
    auto p2 = require_csp0(&env, "a → STOP");
    check_eq(*p1, *p1);
    check_eq(*p1, *p2);
}

TEST_CASE("processes are deduplicated within an environment")
{
    Environment env;
    auto p1 = require_csp0(&env, "a → STOP");
    auto p2 = require_csp0(&env, "a → STOP");
    check_eq(p1, p2);
}

TEST_CASE("can compare sets of processes")
{
    Environment env;
    auto p1 = require_csp0(&env, "a → STOP");
    auto p2 = require_csp0(&env, "a → STOP");
    Process::Set set1{p1};
    Process::Set set2{p2};
    check_eq(set1, set1);
    check_eq(set1, set2);
}

TEST_CASE("process equality considers contents of sets")
{
    Environment env;
    auto p1 = require_csp0(&env, "□ {}");
    auto p2 = require_csp0(&env, "□ {a → STOP}");
    auto p3 = require_csp0(&env, "□ {a → STOP, b → STOP}");
    auto p4 = require_csp0(&env, "□ {a → STOP, b → STOP, c → STOP}");
    check_ne(p1, p2);
    check_ne(p1, p3);
    check_ne(p1, p4);
    check_ne(p2, p3);
    check_ne(p2, p4);
    check_ne(p3, p4);
}

TEST_CASE("process equality considers contents of bags")
{
    Environment env;
    auto p1 = require_csp0(&env, "⫴ {}");
    auto p2 = require_csp0(&env, "⫴ {a → STOP}");
    auto p3 = require_csp0(&env, "⫴ {a → STOP, b → STOP}");
    auto p4 = require_csp0(&env, "⫴ {a → STOP, b → STOP, c → STOP}");
    check_ne(p1, p2);
    check_ne(p1, p3);
    check_ne(p1, p4);
    check_ne(p2, p3);
    check_ne(p2, p4);
    check_ne(p3, p4);
}

TEST_CASE("process equality considers cardinality of contents of bags")
{
    Environment env;
    auto p1 = require_csp0(&env, "⫴ {a→b→STOP, a→b→STOP, a→b→STOP}");
    auto p2 = require_csp0(&env, "⫴ {a→b→STOP, a→b→STOP, b→STOP  }");
    auto p3 = require_csp0(&env, "⫴ {a→b→STOP, b→STOP,   b→STOP  }");
    auto p4 = require_csp0(&env, "⫴ {b→STOP,   b→STOP,   b→STOP  }");
    check_ne(p1, p2);
    check_ne(p1, p3);
    check_ne(p1, p4);
    check_ne(p2, p3);
    check_ne(p2, p4);
    check_ne(p3, p4);
}

TEST_CASE_GROUP("external choice");

TEST_CASE("STOP □ STOP")
{
    auto p = "STOP □ STOP";
    check_name(p, "□ {STOP}");
    check_subprocesses(p, {"STOP"});
    check_initials(p, {});
    check_afters(p, "a", {});
    check_reachable(p, {"STOP □ STOP"});
    check_tau_closure(p, {"STOP □ STOP"});
    check_traces_behavior(p, {});
    check_maximal_traces(p, {{}});
}

TEST_CASE("(a → STOP) □ (b → STOP ⊓ c → STOP)")
{
    auto p = "(a → STOP) □ (b → STOP ⊓ c → STOP)";
    check_name(p, "a → STOP □ (b → STOP ⊓ c → STOP)");
    check_subprocesses(p, {"a → STOP", "b → STOP ⊓ c → STOP"});
    check_initials(p, {"a", "τ"});
    check_afters(p, "a", {"STOP"});
    check_afters(p, "b", {});
    check_afters(p, "τ", {"a → STOP □ b → STOP", "a → STOP □ c → STOP"});
    check_reachable(p, {"(a → STOP) □ (b → STOP ⊓ c → STOP)",
                        "a → STOP □ b → STOP", "a → STOP □ c → STOP", "STOP"});
    check_tau_closure(p, {"(a → STOP) □ (b → STOP ⊓ c → STOP)",
                          "a → STOP □ b → STOP", "a → STOP □ c → STOP"});
    check_traces_behavior(p, {"a"});
    check_maximal_traces(p, {{"a"}, {"b"}, {"c"}});
}

TEST_CASE("(a → STOP) □ (b → STOP)")
{
    auto p = "(a → STOP) □ (b → STOP)";
    check_name(p, "a → STOP □ b → STOP");
    check_subprocesses(p, {"a → STOP", "b → STOP"});
    check_initials(p, {"a", "b"});
    check_afters(p, "a", {"STOP"});
    check_afters(p, "b", {"STOP"});
    check_afters(p, "τ", {});
    check_reachable(p, {"(a → STOP) □ (b → STOP)", "STOP"});
    check_tau_closure(p, {"(a → STOP) □ (b → STOP)"});
    check_traces_behavior(p, {"a", "b"});
    check_maximal_traces(p, {{"a"}, {"b"}});
}

TEST_CASE("□ {a → STOP, b → STOP, c → STOP}")
{
    auto p = "□ {a → STOP, b → STOP, c → STOP}";
    check_name(p, "□ {a → STOP, b → STOP, c → STOP}");
    check_subprocesses(p, {"a → STOP", "b → STOP", "c → STOP"});
    check_initials(p, {"a", "b", "c"});
    check_afters(p, "a", {"STOP"});
    check_afters(p, "b", {"STOP"});
    check_afters(p, "c", {"STOP"});
    check_afters(p, "τ", {});
    check_reachable(p, {"□ {a → STOP, b → STOP, c → STOP}", "STOP"});
    check_tau_closure(p, {"□ {a → STOP, b → STOP, c → STOP}"});
    check_traces_behavior(p, {"a", "b", "c"});
    check_maximal_traces(p, {{"a"}, {"b"}, {"c"}});
}

TEST_CASE_GROUP("interleaving");

TEST_CASE("STOP ⫴ STOP")
{
    auto p = "STOP ⫴ STOP";
    check_name(p, "STOP ⫴ STOP");
    check_subprocesses(p, {"STOP"});
    check_initials(p, {});
    check_afters(p, "a", {});
    check_afters(p, "τ", {});
    check_reachable(p, {"STOP ⫴ STOP"});
    check_tau_closure(p, {"STOP ⫴ STOP"});
    check_traces_behavior(p, {});
    check_maximal_traces(p, {{}});
}

TEST_CASE("Ω ⫴ Ω")
{
    auto p = "Ω ⫴ Ω";
    check_name(p, "Ω ⫴ Ω");
    check_subprocesses(p, {"Ω"});
    check_initials(p, {"✔"});
    check_afters(p, "✔", {"Ω"});
    check_afters(p, "a", {});
    check_afters(p, "τ", {});
    check_reachable(p, {"Ω ⫴ Ω", "Ω"});
    check_tau_closure(p, {"Ω ⫴ Ω"});
    check_traces_behavior(p, {"✔"});
    check_maximal_traces(p, {{"✔"}});
}

TEST_CASE("(a → STOP) ⫴ (b → STOP ⊓ c → STOP)")
{
    auto p = "(a → STOP) ⫴ (b → STOP ⊓ c → STOP)";
    check_name(p, "a → STOP ⫴ b → STOP ⊓ c → STOP");
    check_subprocesses(p, {"a → STOP", "b → STOP ⊓ c → STOP"});
    check_initials(p, {"a", "τ"});
    check_afters(p, "a", {"STOP ⫴ (b → STOP ⊓ c → STOP)"});
    check_afters(p, "b", {});
    check_afters(p, "τ", {"a → STOP ⫴ b → STOP", "a → STOP ⫴ c → STOP"});
    check_reachable(
            p, {"(a → STOP) ⫴ (b → STOP ⊓ c → STOP)",
                "STOP ⫴ (b → STOP ⊓ c → STOP)", "STOP ⫴ b → STOP",
                "STOP ⫴ c → STOP", "a → STOP ⫴ b → STOP", "a → STOP ⫴ c → STOP",
                "a → STOP ⫴ STOP", "STOP ⫴ STOP"});
    check_tau_closure(p, {"(a → STOP) ⫴ (b → STOP ⊓ c → STOP)",
                          "a → STOP ⫴ b → STOP", "a → STOP ⫴ c → STOP"});
    check_traces_behavior(p, {"a"});
    check_maximal_traces(p, {{"a", "b"}, {"a", "c"}, {"b", "a"}, {"c", "a"}});
}

TEST_CASE("a → STOP ⫴ a → STOP")
{
    auto p = "a → STOP ⫴ a → STOP";
    check_name(p, "a → STOP ⫴ a → STOP");
    check_subprocesses(p, {"a → STOP"});
    check_initials(p, {"a"});
    check_afters(p, "a", {"STOP ⫴ a → STOP"});
    check_afters(p, "b", {});
    check_afters(p, "τ", {});
    check_reachable(p,
                    {"a → STOP ⫴ a → STOP", "a → STOP ⫴ STOP", "STOP ⫴ STOP"});
    check_tau_closure(p, {"a → STOP ⫴ a → STOP"});
    check_traces_behavior(p, {"a"});
    check_maximal_traces(p, {{"a", "a"}});
}

TEST_CASE("a → STOP ⫴ b → STOP")
{
    auto p = "a → STOP ⫴ b → STOP";
    check_name(p, "a → STOP ⫴ b → STOP");
    check_subprocesses(p, {"a → STOP", "b → STOP"});
    check_initials(p, {"a", "b"});
    check_afters(p, "a", {"STOP ⫴ b → STOP"});
    check_afters(p, "b", {"a → STOP ⫴ STOP"});
    check_afters(p, "τ", {});
    check_reachable(p, {"a → STOP ⫴ b → STOP", "a → STOP ⫴ STOP",
                        "STOP ⫴ b → STOP", "STOP ⫴ STOP"});
    check_tau_closure(p, {"a → STOP ⫴ b → STOP"});
    check_traces_behavior(p, {"a", "b"});
    check_maximal_traces(p, {{"a", "b"}, {"b", "a"}});
}

TEST_CASE("a → SKIP ⫴ b → SKIP")
{
    auto p = "a → SKIP ⫴ b → SKIP";
    check_name(p, "a → SKIP ⫴ b → SKIP");
    check_subprocesses(p, {"a → SKIP", "b → SKIP"});
    check_initials(p, {"a", "b"});
    check_afters(p, "a", {"SKIP ⫴ b → SKIP"});
    check_afters(p, "b", {"a → SKIP ⫴ SKIP"});
    check_afters(p, "τ", {});
    check_afters(p, "✔", {});
    check_reachable(p, {"a → SKIP ⫴ b → SKIP", "a → SKIP ⫴ SKIP",
                        "a → SKIP ⫴ Ω", "SKIP ⫴ b → SKIP", "Ω ⫴ b → SKIP",
                        "Ω ⫴ SKIP", "Ω ⫴ Ω", "SKIP ⫴ SKIP", "Ω"});
    check_tau_closure(p, {"a → SKIP ⫴ b → SKIP"});
    check_traces_behavior(p, {"a", "b"});
    check_maximal_traces(p, {{"a", "b", "✔"}, {"b", "a", "✔"}});
}

TEST_CASE("(a → SKIP ⫴ b → SKIP) ; c → STOP")
{
    auto p = "(a → SKIP ⫴ b → SKIP) ; c → STOP";
    check_name(p, "(a → SKIP ⫴ b → SKIP) ; c → STOP");
    check_subprocesses(p, {"a → SKIP ⫴ b → SKIP", "c → STOP"});
    check_initials(p, {"a", "b"});
    check_afters(p, "a", {"(SKIP ⫴ b → SKIP) ; c → STOP"});
    check_afters(p, "b", {"(a → SKIP ⫴ SKIP) ; c → STOP"});
    check_afters(p, "τ", {});
    check_reachable(
            p, {"(a → SKIP ⫴ b → SKIP) ; c → STOP",
                "(a → SKIP ⫴ SKIP) ; c → STOP", "(a → SKIP ⫴ Ω) ; c → STOP",
                "(SKIP ⫴ b → SKIP) ; c → STOP", "(Ω ⫴ b → SKIP) ; c → STOP",
                "(Ω ⫴ SKIP) ; c → STOP", "(Ω ⫴ Ω) ; c → STOP",
                "(SKIP ⫴ SKIP) ; c → STOP", "c → STOP", "STOP"});
    check_tau_closure(p, {"(a → SKIP ⫴ b → SKIP) ; c → STOP"});
    check_traces_behavior(p, {"a", "b"});
    check_maximal_traces(p, {{"a", "b", "c"}, {"b", "a", "c"}});
}

TEST_CASE("⫴ {a → STOP, b → STOP, c → STOP}")
{
    auto p = "⫴ {a → STOP, b → STOP, c → STOP}";
    check_name(p, "⫴ {a → STOP, b → STOP, c → STOP}");
    check_subprocesses(p, {"a → STOP", "b → STOP", "c → STOP"});
    check_initials(p, {"a", "b", "c"});
    check_afters(p, "a", {"⫴ {STOP, b → STOP, c → STOP}"});
    check_afters(p, "b", {"⫴ {a → STOP, STOP, c → STOP}"});
    check_afters(p, "c", {"⫴ {a → STOP, b → STOP, STOP}"});
    check_afters(p, "τ", {});
    check_reachable(
            p,
            {"⫴ {a → STOP, b → STOP, c → STOP}", "⫴ {STOP, a → STOP, b → STOP}",
             "⫴ {STOP, a → STOP, c → STOP}", "⫴ {STOP, b → STOP, c → STOP}",
             "⫴ {STOP, STOP, a → STOP}", "⫴ {STOP, STOP, b → STOP}",
             "⫴ {STOP, STOP, c → STOP}", "⫴ {STOP, STOP, STOP}"});
    check_tau_closure(p, {"⫴ {a → STOP, b → STOP, c → STOP}"});
    check_traces_behavior(p, {"a", "b", "c"});
    check_maximal_traces(p, {{"a", "b", "c"},
                             {"a", "c", "b"},
                             {"b", "a", "c"},
                             {"b", "c", "a"},
                             {"c", "a", "b"},
                             {"c", "b", "a"}});
}

TEST_CASE_GROUP("internal choice");

TEST_CASE("STOP ⊓ STOP")
{
    auto p = "STOP ⊓ STOP";
    check_name(p, "⊓ {STOP}");
    check_subprocesses(p, {"STOP"});
    check_initials(p, {"τ"});
    check_afters(p, "τ", {"STOP"});
    check_afters(p, "a", {});
    check_reachable(p, {"STOP ⊓ STOP", "STOP"});
    check_tau_closure(p, {"STOP ⊓ STOP", "STOP"});
    check_traces_behavior(p, {});
    check_maximal_traces(p, {{}});
}

TEST_CASE("(a → STOP) ⊓ (b → STOP)")
{
    auto p = "(a → STOP) ⊓ (b → STOP)";
    check_name(p, "a → STOP ⊓ b → STOP");
    check_subprocesses(p, {"a → STOP", "b → STOP"});
    check_initials(p, {"τ"});
    check_afters(p, "τ", {"a → STOP", "b → STOP"});
    check_afters(p, "a", {});
    check_reachable(
            p, {"(a → STOP) ⊓ (b → STOP)", "a → STOP", "b → STOP", "STOP"});
    check_tau_closure(p, {"(a → STOP) ⊓ (b → STOP)", "a → STOP", "b → STOP"});
    check_traces_behavior(p, {});
    check_maximal_traces(p, {{"a"}, {"b"}});
}

TEST_CASE("⊓ {a → STOP, b → STOP, c → STOP}")
{
    auto p = "⊓ {a → STOP, b → STOP, c → STOP}";
    check_name(p, "⊓ {a → STOP, b → STOP, c → STOP}");
    check_subprocesses(p, {"a → STOP", "b → STOP", "c → STOP"});
    check_initials(p, {"τ"});
    check_afters(p, "τ", {"a → STOP", "b → STOP", "c → STOP"});
    check_afters(p, "a", {});
    check_reachable(p, {"⊓ {a → STOP, b → STOP, c → STOP}", "a → STOP",
                        "b → STOP", "c → STOP", "STOP"});
    check_tau_closure(p, {"⊓ {a → STOP, b → STOP, c → STOP}", "a → STOP",
                          "b → STOP", "c → STOP"});
    check_traces_behavior(p, {});
    check_maximal_traces(p, {{"a"}, {"b"}, {"c"}});
}

TEST_CASE_GROUP("prefix");

TEST_CASE("a → STOP")
{
    auto p = "a → STOP";
    check_name(p, "a → STOP");
    check_subprocesses(p, {"STOP"});
    check_initials(p, {"a"});
    check_afters(p, "a", {"STOP"});
    check_afters(p, "τ", {});
    check_reachable(p, {"a → STOP", "STOP"});
    check_tau_closure(p, {"a → STOP"});
    check_traces_behavior(p, {"a"});
    check_maximal_traces(p, {{"a"}});
}

TEST_CASE("a → b → STOP")
{
    auto p = "a → b → STOP";
    check_name(p, "a → b → STOP");
    check_subprocesses(p, {"b → STOP"});
    check_initials(p, {"a"});
    check_afters(p, "a", {"b → STOP"});
    check_afters(p, "τ", {});
    check_reachable(p, {"a → b → STOP", "b → STOP", "STOP"});
    check_tau_closure(p, {"a → b → STOP"});
    check_traces_behavior(p, {"a"});
    check_maximal_traces(p, {{"a", "b"}});
}

TEST_CASE_GROUP("recursion");

TEST_CASE("let X=a → STOP within X")
{
    auto p = "let X=a → STOP within X";
    check_name(p, "let X=a → STOP within X");
    check_subprocesses(p, {"a → STOP"});
    check_initials(p, {"a"});
    check_afters(p, "a", {"STOP"});
    check_reachable(p, {"X@0", "STOP"});
    check_tau_closure(p, {"X@0"});
    check_traces_behavior(p, {"a"});
    check_maximal_traces(p, {{"a"}});
}

TEST_CASE("let X=a → Y Y=b → X within X")
{
    auto p = "let X=a → Y Y=b → X within X";
    check_name(p, "let X=a → Y Y=b → X within X");
    check_subprocesses(p, {"a → Y@0"});
    check_initials(p, {"a"});
    check_afters(p, "a", {"Y@0"});
    check_reachable(p, {"X@0", "Y@0"});
    check_tau_closure(p, {"X@0"});
    check_traces_behavior(p, {"a"});
    check_maximal_traces(p, {{"a", "b"}});
}

TEST_CASE("let X=Y □ Z Y=a → X Z=b → X within X")
{
    auto p = "let X=Y □ Z Y=a → X Z=b → X within X";
    check_name(p, "let X=Y □ Z Y=a → X Z=b → X within X");
    check_subprocesses(p, {"Y@0 □ Z@0"});
    check_initials(p, {"a", "b"});
    check_afters(p, "a", {"X@0"});
    check_afters(p, "b", {"X@0"});
    // Y@0 and Z@0 aren't reachable because the external choice "pulls them
    // apart" when calculating X@0's initials and afters.
    check_reachable(p, {"X@0"});
    check_tau_closure(p, {"X@0"});
    check_traces_behavior(p, {"a", "b"});
    check_maximal_traces(p, {{"a"}, {"b"}});
}

TEST_CASE_GROUP("SKIP");

TEST_CASE("SKIP")
{
    auto skip = "SKIP";
    check_name(skip, "SKIP");
    check_subprocesses(skip, {});
    check_initials(skip, {"✔"});
    check_afters(skip, "a", {});
    check_afters(skip, "τ", {});
    check_afters(skip, "✔", {"Ω"});
    check_reachable(skip, {"SKIP", "Ω"});
    check_tau_closure(skip, {"SKIP"});
    check_traces_behavior(skip, {"✔"});
    check_maximal_traces(skip, {{"✔"}});
}

TEST_CASE_GROUP("STOP");

TEST_CASE("STOP")
{
    auto stop = "STOP";
    check_name(stop, "STOP");
    check_subprocesses(stop, {});
    check_initials(stop, {});
    check_afters(stop, "a", {});
    check_afters(stop, "τ", {});
    check_reachable(stop, {"STOP"});
    check_tau_closure(stop, {"STOP"});
    check_traces_behavior(stop, {});
    check_maximal_traces(stop, {{}});
}

TEST_CASE_GROUP("sequential composition");

TEST_CASE("SKIP ; STOP")
{
    auto p = "SKIP ; STOP";
    check_name(p, "SKIP ; STOP");
    check_subprocesses(p, {"SKIP", "STOP"});
    check_initials(p, {"τ"});
    check_afters(p, "a", {});
    check_afters(p, "b", {});
    check_afters(p, "τ", {"STOP"});
    check_afters(p, "✔", {});
    check_reachable(p, {"SKIP ; STOP", "STOP"});
    check_tau_closure(p, {"SKIP ; STOP", "STOP"});
    check_traces_behavior(p, {});
    check_maximal_traces(p, {{}});
}

TEST_CASE("a → SKIP ; STOP")
{
    auto p = "a → SKIP ; STOP";
    check_name(p, "a → SKIP ; STOP");
    check_subprocesses(p, {"a → SKIP", "STOP"});
    check_initials(p, {"a"});
    check_afters(p, "a", {"SKIP ; STOP"});
    check_afters(p, "b", {});
    check_afters(p, "τ", {});
    check_afters(p, "✔", {});
    check_reachable(p, {"a → SKIP ; STOP", "SKIP ; STOP", "STOP"});
    check_tau_closure(p, {"a → SKIP ; STOP"});
    check_traces_behavior(p, {"a"});
    check_maximal_traces(p, {{"a"}});
}

TEST_CASE("(a → b → STOP □ SKIP) ; STOP")
{
    auto p = "(a → b → STOP □ SKIP) ; STOP";
    check_name(p, "(SKIP □ a → b → STOP) ; STOP");
    check_subprocesses(p, {"a → b → STOP □ SKIP", "STOP"});
    check_initials(p, {"a", "τ"});
    check_afters(p, "a", {"b → STOP ; STOP"});
    check_afters(p, "b", {});
    check_afters(p, "τ", {"STOP"});
    check_afters(p, "✔", {});
    check_reachable(p, {"(a → b → STOP □ SKIP) ; STOP", "b → STOP ; STOP",
                        "STOP ; STOP", "STOP"});
    check_tau_closure(p, {"(a → b → STOP □ SKIP) ; STOP", "STOP"});
    check_traces_behavior(p, {"a"});
    check_maximal_traces(p, {{"a", "b"}});
}

TEST_CASE("(a → b → STOP ⊓ SKIP) ; STOP")
{
    auto p = "(a → b → STOP ⊓ SKIP) ; STOP";
    check_name(p, "(SKIP ⊓ a → b → STOP) ; STOP");
    check_subprocesses(p, {"a → b → STOP ⊓ SKIP", "STOP"});
    check_initials(p, {"τ"});
    check_afters(p, "a", {});
    check_afters(p, "b", {});
    check_afters(p, "τ", {"a → b → STOP ; STOP", "SKIP ; STOP"});
    check_afters(p, "✔", {});
    check_reachable(p,
                    {"(a → b → STOP ⊓ SKIP) ; STOP", "a → b → STOP ; STOP",
                     "SKIP ; STOP", "b → STOP ; STOP", "STOP ; STOP", "STOP"});
    check_tau_closure(p, {"(a → b → STOP ⊓ SKIP) ; STOP", "a → b → STOP ; STOP",
                          "SKIP ; STOP", "STOP"});
    check_traces_behavior(p, {});
    check_maximal_traces(p, {{"a", "b"}});
}

TEST_CASE_GROUP("prenormalization");

TEST_CASE("prenormalize {a → STOP}")
{
    auto p = "prenormalize {a → STOP}";
    check_name(p, "prenormalize {a → STOP}");
    check_subprocesses(p, {"a → STOP"});
    check_initials(p, {"a"});
    check_afters(p, "a", {"prenormalize {STOP}"});
    check_afters(p, "τ", {});
    check_reachable(p, {"prenormalize {a → STOP}", "prenormalize {STOP}"});
    check_tau_closure(p, {"prenormalize {a → STOP}"});
    check_traces_behavior(p, {"a"});
    check_maximal_traces(p, {{"a"}});
    check_expansion(p, {"a → STOP"});
}

TEST_CASE("prenormalize {a → STOP □ b → STOP}")
{
    auto p = "prenormalize {a → STOP □ b → STOP}";
    check_name(p, "prenormalize {a → STOP □ b → STOP}");
    check_subprocesses(p, {"a → STOP □ b → STOP"});
    check_initials(p, {"a", "b"});
    check_afters(p, "a", {"prenormalize {STOP}"});
    check_afters(p, "b", {"prenormalize {STOP}"});
    check_afters(p, "τ", {});
    check_reachable(
            p, {"prenormalize {a → STOP □ b → STOP}", "prenormalize {STOP}"});
    check_tau_closure(p, {"prenormalize {a → STOP □ b → STOP}"});
    check_traces_behavior(p, {"a", "b"});
    check_maximal_traces(p, {{"a"}, {"b"}});
    check_expansion(p, {"a → STOP □ b → STOP"});
}

TEST_CASE("prenormalize {a → STOP □ a → b → STOP}")
{
    auto p = "prenormalize {a → STOP □ a → b → STOP}";
    check_name(p, "prenormalize {a → STOP □ a → b → STOP}");
    check_subprocesses(p, {"a → STOP □ a → b → STOP"});
    check_initials(p, {"a"});
    check_afters(p, "a", {"prenormalize {STOP, b → STOP}"});
    check_afters(p, "τ", {});
    check_reachable(p,
                    {"prenormalize {a → STOP □ a → b → STOP}",
                     "prenormalize {STOP, b → STOP}", "prenormalize {STOP}"});
    check_tau_closure(p, {"prenormalize {a → STOP □ a → b → STOP}"});
    check_traces_behavior(p, {"a"});
    check_maximal_traces(p, {{"a", "b"}});
    check_expansion(p, {"a → STOP □ a → b → STOP"});
}

TEST_CASE("prenormalize {a → STOP ⊓ b → STOP}")
{
    auto p = "prenormalize {a → STOP ⊓ b → STOP}";
    check_name(p, "prenormalize {a → STOP, b → STOP, a → STOP ⊓ b → STOP}");
    check_subprocesses(p, {"a → STOP ⊓ b → STOP", "a → STOP", "b → STOP"});
    check_initials(p, {"a", "b"});
    check_afters(p, "a", {"prenormalize {STOP}"});
    check_afters(p, "b", {"prenormalize {STOP}"});
    check_afters(p, "τ", {});
    check_reachable(
            p, {"prenormalize {a → STOP ⊓ b → STOP}", "prenormalize {STOP}"});
    check_tau_closure(p, {"prenormalize {a → STOP ⊓ b → STOP}"});
    check_traces_behavior(p, {"a", "b"});
    check_maximal_traces(p, {{"a"}, {"b"}});
    check_expansion(p, {"a → STOP ⊓ b → STOP", "a → STOP", "b → STOP"});
}

TEST_CASE("prenormalize {a → SKIP ; b → STOP}")
{
    auto p = "prenormalize {a → SKIP ; b → STOP}";
    check_name(p, "prenormalize {a → SKIP ; b → STOP}");
    check_subprocesses(p, {"a → SKIP ; b → STOP"});
    check_initials(p, {"a"});
    check_afters(p, "a", {"prenormalize {SKIP ; b → STOP}"});
    check_afters(p, "τ", {});
    check_reachable(p,
                    {"prenormalize {a → SKIP ; b → STOP}",
                     "prenormalize {SKIP ; b → STOP}", "prenormalize {STOP}"});
    check_tau_closure(p, {"prenormalize {a → SKIP ; b → STOP}"});
    check_traces_behavior(p, {"a"});
    check_maximal_traces(p, {{"a", "b"}});
    check_expansion(p, {"a → SKIP ; b → STOP"});
}

TEST_CASE_GROUP("normalization");

TEST_CASE("normalize[T] {a → STOP}")
{
    auto p = "normalize[T] {a → STOP}";
    check_name(p, "normalize[T] {a → STOP}");
    check_initials(p, {"a"});
    check_afters(p, "a", {"normalize[T] {STOP} within {a → STOP}"});
    check_afters(p, "τ", {});
    check_reachable(p, {"normalize[T] {a → STOP} within {a → STOP}",
                        "normalize[T] {STOP} within {a → STOP}"});
    check_tau_closure(p, {"normalize[T] {a → STOP} within {a → STOP}"});
    check_traces_behavior(p, {"a"});
    check_maximal_traces(p, {{"a"}});
    check_expansion(p, {"a → STOP"});
}

TEST_CASE("normalize[T] {b → a → a → STOP □ c → a → a → STOP} (using let)")
{
    // We want to use a let here instead of using □ directly like in the test
    // name, because our memoization would make the two copies of "a→a→STOP"
    // refer to exactly the same Process instance.  We want to make sure that we
    // have separate Process instances that have the same behavior; hence the
    // lets
    auto p = "normalize[T] {"
             "  let "
             "    root=b → A □ c → D "
             "    A=□ {a → B} "
             "    B=□ {a → C} "
             "    C=□ {} "
             "    D=□ {a → E} "
             "    E=□ {a → F} "
             "    F=□ {} "
             "  within root"
             "}";
    check_name(p,
               "normalize[T] {"
               "let "
               "root=b → A □ c → D "
               "A=□ {a → B} "  // This order because we render the definitions
               "D=□ {a → E} "  // in the order that they're encountered, and we
               "B=□ {a → C} "  // reference A and D up in root before we start
               "C=□ {} "       // defining any of the subprocesses.
               "E=□ {a → F} "
               "F=□ {} "
               "within root"
               "}");
    check_initials(p, {"b", "c"});
    // Since A and D have the same behavior (even though we've ensured that
    // they're distinct Process objects), they're merged together during
    // bisimulation.
    check_afters(p, "b", {"normalize[T] {A@0,D@0} within {root@0}"});
    check_afters(p, "c", {"normalize[T] {A@0,D@0} within {root@0}"});
    check_afters(p, "τ", {});
    check_reachable(p, {"normalize[T] {root@0}",
                        "normalize[T] {A@0,D@0} within {root@0}",
                        "normalize[T] {B@0,E@0} within {root@0}",
                        "normalize[T] {C@0,F@0} within {root@0}"});
    check_tau_closure(p, {"normalize[T] {root@0}"});
    check_traces_behavior(p, {"b", "c"});
    check_maximal_traces(p, {{"b", "a", "a"}, {"c", "a", "a"}});
    check_expansion(p, {"root@0"});
}

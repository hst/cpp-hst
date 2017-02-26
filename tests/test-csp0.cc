/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/csp0.h"

#include <string>

#include "test-cases.h"
#include "test-harness.cc.in"

#include "hst/environment.h"
#include "hst/event.h"
#include "hst/process.h"

// Don't check the semantics of any of the operators here; this file just checks
// that the CSP₀ parser produces the same processes that you'd get constructing
// things by hand.  Look in test-operators.cc for test cases that verify that
// each operator behaves as we expect it to.

using hst::Environment;
using hst::Event;
using hst::ParseError;
using hst::Process;

static void
check_csp0_valid(const std::string& csp0)
{
    Environment env;
    ParseError error;
    if (!hst::load_csp0_string(&env, csp0, &error)) {
        fail() << "Could not parse " << csp0 << ": " << error << abort_test();
    }
}

static void
check_csp0_invalid(const std::string& csp0)
{
    Environment env;
    ParseError error;
    if (hst::load_csp0_string(&env, csp0, &error)) {
        fail() << "Shouldn't be able to parse " << csp0 << abort_test();
    }
}

static void
check_csp0_eq(Environment* env, const std::string& csp0,
              const Process* expected)
{
    ParseError error;
    const Process* actual = hst::load_csp0_string(env, csp0, &error);
    if (!actual) {
        fail() << "Could not parse " << csp0 << ": " << error << abort_test();
    }
    check_eq(*actual, *expected);
}

TEST_CASE_GROUP("CSP₀ syntax");

TEST_CASE("can parse identifiers")
{
    // Parse a bunch of valid identifiers.
    check_csp0_valid("r → STOP");
    check_csp0_valid("r0 → STOP");
    check_csp0_valid("r0r → STOP");
    check_csp0_valid("root → STOP");
    check_csp0_valid("root.root → STOP");
    check_csp0_valid("root_root → STOP");
    check_csp0_valid("_ → STOP");
    check_csp0_valid("_r → STOP");
    check_csp0_valid("_r0 → STOP");
    check_csp0_valid("_r0r → STOP");
    check_csp0_valid("_root → STOP");
    check_csp0_valid("_root.root → STOP");
    check_csp0_valid("_root_root → STOP");
    check_csp0_valid("$r → STOP");
    check_csp0_valid("$r0 → STOP");
    check_csp0_valid("$r0r → STOP");
    check_csp0_valid("$root → STOP");
    check_csp0_valid("$root.root → STOP");
    check_csp0_valid("$root_root → STOP");
    // Fail to parse a bunch of invalid identifiers.
    check_csp0_invalid("0 → STOP");
    check_csp0_invalid("$ → STOP");
}

TEST_CASE_GROUP("CSP₀ primitives");

TEST_CASE("parse: STOP")
{
    Environment env;
    auto expected = env.stop();
    check_csp0_eq(&env, "STOP", expected);
    check_csp0_eq(&env, " STOP", expected);
    check_csp0_eq(&env, "STOP ", expected);
    check_csp0_eq(&env, " STOP ", expected);
}

TEST_CASE("parse: SKIP")
{
    Environment env;
    auto expected = env.skip();
    check_csp0_eq(&env, "SKIP", expected);
    check_csp0_eq(&env, " SKIP", expected);
    check_csp0_eq(&env, "SKIP ", expected);
    check_csp0_eq(&env, " SKIP ", expected);
}

TEST_CASE_GROUP("CSP₀ operators");

TEST_CASE("parse: a → STOP □ SKIP")
{
    Environment env;
    auto expected =
            env.external_choice(env.prefix(Event("a"), env.stop()), env.skip());
    check_csp0_eq(&env, "a->STOP[]SKIP", expected);
    check_csp0_eq(&env, " a->STOP[]SKIP", expected);
    check_csp0_eq(&env, " a ->STOP[]SKIP", expected);
    check_csp0_eq(&env, " a -> STOP[]SKIP", expected);
    check_csp0_eq(&env, " a -> STOP []SKIP", expected);
    check_csp0_eq(&env, " a -> STOP [] SKIP", expected);
    check_csp0_eq(&env, " a -> STOP [] SKIP ", expected);
    check_csp0_eq(&env, "a→STOP□SKIP", expected);
    check_csp0_eq(&env, " a→STOP□SKIP", expected);
    check_csp0_eq(&env, " a →STOP□SKIP", expected);
    check_csp0_eq(&env, " a → STOP□SKIP", expected);
    check_csp0_eq(&env, " a → STOP □SKIP", expected);
    check_csp0_eq(&env, " a → STOP □ SKIP", expected);
    check_csp0_eq(&env, " a → STOP □ SKIP ", expected);
    // Fail to parse a bunch of invalid statements.
    // a is undefined
    check_csp0_invalid("a □ STOP");
    check_csp0_invalid("STOP □ a");
}

TEST_CASE("associativity: a → STOP □ b → STOP □ c → STOP")
{
    Environment env;
    auto expected = env.external_choice(
            env.prefix(Event("a"), env.stop()),
            env.external_choice(env.prefix(Event("b"), env.stop()),
                                env.prefix(Event("c"), env.stop())));
    check_csp0_eq(&env, "a -> STOP [] b -> STOP [] c -> STOP", expected);
    check_csp0_eq(&env, "a → STOP □ b → STOP □ c → STOP", expected);
}

TEST_CASE("parse: a → STOP ⫴ SKIP")
{
    Environment env;
    auto expected =
            env.interleave(env.prefix(Event("a"), env.stop()), env.skip());
    check_csp0_eq(&env, "a->STOP|||SKIP", expected);
    check_csp0_eq(&env, " a->STOP|||SKIP", expected);
    check_csp0_eq(&env, " a ->STOP|||SKIP", expected);
    check_csp0_eq(&env, " a -> STOP|||SKIP", expected);
    check_csp0_eq(&env, " a -> STOP |||SKIP", expected);
    check_csp0_eq(&env, " a -> STOP ||| SKIP", expected);
    check_csp0_eq(&env, " a -> STOP ||| SKIP ", expected);
    check_csp0_eq(&env, "a→STOP⫴SKIP", expected);
    check_csp0_eq(&env, " a→STOP⫴SKIP", expected);
    check_csp0_eq(&env, " a →STOP⫴SKIP", expected);
    check_csp0_eq(&env, " a → STOP⫴SKIP", expected);
    check_csp0_eq(&env, " a → STOP ⫴SKIP", expected);
    check_csp0_eq(&env, " a → STOP ⫴ SKIP", expected);
    check_csp0_eq(&env, " a → STOP ⫴ SKIP ", expected);
    // Fail to parse a bunch of invalid statements.
    // a is undefined
    check_csp0_invalid("a ⫴ STOP");
    check_csp0_invalid("STOP ⫴ a");
}

TEST_CASE("associativity: a → STOP ⫴ b → STOP ⫴ c → STOP")
{
    Environment env;
    auto expected =
            env.interleave(env.prefix(Event("a"), env.stop()),
                           env.interleave(env.prefix(Event("b"), env.stop()),
                                          env.prefix(Event("c"), env.stop())));
    check_csp0_eq(&env, "a -> STOP ||| b -> STOP ||| c -> STOP", expected);
    check_csp0_eq(&env, "a → STOP ⫴ b → STOP ⫴ c → STOP", expected);
}

TEST_CASE("parse: a → STOP ⊓ SKIP")
{
    Environment env;
    auto expected =
            env.internal_choice(env.prefix(Event("a"), env.stop()), env.skip());
    check_csp0_eq(&env, "a->STOP|~|SKIP", expected);
    check_csp0_eq(&env, " a->STOP|~|SKIP", expected);
    check_csp0_eq(&env, " a ->STOP|~|SKIP", expected);
    check_csp0_eq(&env, " a -> STOP|~|SKIP", expected);
    check_csp0_eq(&env, " a -> STOP |~|SKIP", expected);
    check_csp0_eq(&env, " a -> STOP |~| SKIP", expected);
    check_csp0_eq(&env, " a -> STOP |~| SKIP ", expected);
    check_csp0_eq(&env, "a→STOP⊓SKIP", expected);
    check_csp0_eq(&env, " a→STOP⊓SKIP", expected);
    check_csp0_eq(&env, " a →STOP⊓SKIP", expected);
    check_csp0_eq(&env, " a → STOP⊓SKIP", expected);
    check_csp0_eq(&env, " a → STOP ⊓SKIP", expected);
    check_csp0_eq(&env, " a → STOP ⊓ SKIP", expected);
    check_csp0_eq(&env, " a → STOP ⊓ SKIP ", expected);
    // Fail to parse a bunch of invalid statements.
    // a is undefined
    check_csp0_invalid("a ⊓ STOP");
    check_csp0_invalid("STOP ⊓ a");
}

TEST_CASE("associativity: a → STOP ⊓ b → STOP ⊓ c → STOP")
{
    Environment env;
    auto expected = env.internal_choice(
            env.prefix(Event("a"), env.stop()),
            env.internal_choice(env.prefix(Event("b"), env.stop()),
                                env.prefix(Event("c"), env.stop())));
    check_csp0_eq(&env, "a -> STOP |~| b -> STOP |~| c -> STOP", expected);
    check_csp0_eq(&env, "a → STOP ⊓ b → STOP ⊓ c → STOP", expected);
}

TEST_CASE("parse: (STOP)")
{
    Environment env;
    auto expected = env.stop();
    check_csp0_eq(&env, "(STOP)", expected);
    check_csp0_eq(&env, " (STOP)", expected);
    check_csp0_eq(&env, " ( STOP)", expected);
    check_csp0_eq(&env, " ( STOP )", expected);
    check_csp0_eq(&env, " ( STOP ) ", expected);
    check_csp0_eq(&env, "((STOP))", expected);
    check_csp0_eq(&env, "(((STOP)))", expected);
}

TEST_CASE("parse: a → STOP")
{
    Environment env;
    auto expected = env.prefix(Event("a"), env.stop());
    check_csp0_eq(&env, "a->STOP", expected);
    check_csp0_eq(&env, " a->STOP", expected);
    check_csp0_eq(&env, " a ->STOP", expected);
    check_csp0_eq(&env, " a -> STOP", expected);
    check_csp0_eq(&env, " a -> STOP ", expected);
    check_csp0_eq(&env, "a→STOP", expected);
    check_csp0_eq(&env, " a→STOP", expected);
    check_csp0_eq(&env, " a →STOP", expected);
    check_csp0_eq(&env, " a → STOP", expected);
    check_csp0_eq(&env, " a → STOP ", expected);
    // Fail to parse a bunch of invalid statements.
    // STOP isn't an event
    check_csp0_invalid("STOP → STOP");
    // undefined isn't defined
    check_csp0_invalid("a → undefined");
    // b isn't a process (reported as "b is undefined")
    check_csp0_invalid("(a → b) → STOP");
}

TEST_CASE("associativity: a → b → STOP")
{
    Environment env;
    auto expected = env.prefix(Event("a"), env.prefix(Event("b"), env.stop()));
    check_csp0_eq(&env, "a -> b -> STOP", expected);
    check_csp0_eq(&env, "a → b → STOP", expected);
}

TEST_CASE("parse: □ {a → STOP, SKIP}")
{
    Environment env;
    auto expected =
            env.external_choice(env.prefix(Event("a"), env.stop()), env.skip());
    check_csp0_eq(&env, "[]{a->STOP,SKIP}", expected);
    check_csp0_eq(&env, " []{a->STOP,SKIP}", expected);
    check_csp0_eq(&env, " [] {a->STOP,SKIP}", expected);
    check_csp0_eq(&env, " [] { a->STOP,SKIP}", expected);
    check_csp0_eq(&env, " [] { a ->STOP,SKIP}", expected);
    check_csp0_eq(&env, " [] { a -> STOP,SKIP}", expected);
    check_csp0_eq(&env, " [] { a -> STOP ,SKIP}", expected);
    check_csp0_eq(&env, " [] { a -> STOP , SKIP}", expected);
    check_csp0_eq(&env, " [] { a -> STOP , SKIP }", expected);
    check_csp0_eq(&env, " [] { a -> STOP , SKIP } ", expected);
    check_csp0_eq(&env, "□{a→STOP,SKIP}", expected);
    check_csp0_eq(&env, " □{a→STOP,SKIP}", expected);
    check_csp0_eq(&env, " □ {a→STOP,SKIP}", expected);
    check_csp0_eq(&env, " □ { a→STOP,SKIP}", expected);
    check_csp0_eq(&env, " □ { a →STOP,SKIP}", expected);
    check_csp0_eq(&env, " □ { a → STOP,SKIP}", expected);
    check_csp0_eq(&env, " □ { a → STOP ,SKIP}", expected);
    check_csp0_eq(&env, " □ { a → STOP , SKIP}", expected);
    check_csp0_eq(&env, " □ { a → STOP , SKIP }", expected);
    // missing `{`
    check_csp0_invalid("□");
    // missing process after `{`
    check_csp0_invalid("□ {");
    // missing `}`
    check_csp0_invalid("□ { STOP");
    // missing process after `,`
    check_csp0_invalid("□ { STOP,");
    check_csp0_invalid("□ { STOP, }");
    // a is undefined
    check_csp0_invalid("□ { a, STOP }");
    check_csp0_invalid("□ { STOP, a }");
}

TEST_CASE("parse: ⫴ {a → STOP, SKIP}")
{
    Environment env;
    auto expected =
            env.interleave(env.prefix(Event("a"), env.stop()), env.skip());
    check_csp0_eq(&env, "|||{a->STOP,SKIP}", expected);
    check_csp0_eq(&env, " |||{a->STOP,SKIP}", expected);
    check_csp0_eq(&env, " ||| {a->STOP,SKIP}", expected);
    check_csp0_eq(&env, " ||| { a->STOP,SKIP}", expected);
    check_csp0_eq(&env, " ||| { a ->STOP,SKIP}", expected);
    check_csp0_eq(&env, " ||| { a -> STOP,SKIP}", expected);
    check_csp0_eq(&env, " ||| { a -> STOP ,SKIP}", expected);
    check_csp0_eq(&env, " ||| { a -> STOP , SKIP}", expected);
    check_csp0_eq(&env, " ||| { a -> STOP , SKIP }", expected);
    check_csp0_eq(&env, " ||| { a -> STOP , SKIP } ", expected);
    check_csp0_eq(&env, "⫴{a→STOP,SKIP}", expected);
    check_csp0_eq(&env, " ⫴{a→STOP,SKIP}", expected);
    check_csp0_eq(&env, " ⫴ {a→STOP,SKIP}", expected);
    check_csp0_eq(&env, " ⫴ { a→STOP,SKIP}", expected);
    check_csp0_eq(&env, " ⫴ { a →STOP,SKIP}", expected);
    check_csp0_eq(&env, " ⫴ { a → STOP,SKIP}", expected);
    check_csp0_eq(&env, " ⫴ { a → STOP ,SKIP}", expected);
    check_csp0_eq(&env, " ⫴ { a → STOP , SKIP}", expected);
    check_csp0_eq(&env, " ⫴ { a → STOP , SKIP }", expected);
    // missing `{`
    check_csp0_invalid("⫴");
    // missing process after `{`
    check_csp0_invalid("⫴ {");
    // missing `}`
    check_csp0_invalid("⫴ { STOP");
    // missing process after `,`
    check_csp0_invalid("⫴ { STOP,");
    check_csp0_invalid("⫴ { STOP, }");
    // a is undefined
    check_csp0_invalid("⫴ { a, STOP }");
    check_csp0_invalid("⫴ { STOP, a }");
}

TEST_CASE("parse: ⊓ {a → STOP, SKIP}")
{
    Environment env;
    auto expected =
            env.internal_choice(env.prefix(Event("a"), env.stop()), env.skip());
    check_csp0_eq(&env, "|~|{a->STOP,SKIP}", expected);
    check_csp0_eq(&env, " |~|{a->STOP,SKIP}", expected);
    check_csp0_eq(&env, " |~| {a->STOP,SKIP}", expected);
    check_csp0_eq(&env, " |~| { a->STOP,SKIP}", expected);
    check_csp0_eq(&env, " |~| { a ->STOP,SKIP}", expected);
    check_csp0_eq(&env, " |~| { a -> STOP,SKIP}", expected);
    check_csp0_eq(&env, " |~| { a -> STOP ,SKIP}", expected);
    check_csp0_eq(&env, " |~| { a -> STOP , SKIP}", expected);
    check_csp0_eq(&env, " |~| { a -> STOP , SKIP }", expected);
    check_csp0_eq(&env, " |~| { a -> STOP , SKIP } ", expected);
    check_csp0_eq(&env, "⊓{a→STOP,SKIP}", expected);
    check_csp0_eq(&env, " ⊓{a→STOP,SKIP}", expected);
    check_csp0_eq(&env, " ⊓ {a→STOP,SKIP}", expected);
    check_csp0_eq(&env, " ⊓ { a→STOP,SKIP}", expected);
    check_csp0_eq(&env, " ⊓ { a →STOP,SKIP}", expected);
    check_csp0_eq(&env, " ⊓ { a → STOP,SKIP}", expected);
    check_csp0_eq(&env, " ⊓ { a → STOP ,SKIP}", expected);
    check_csp0_eq(&env, " ⊓ { a → STOP , SKIP}", expected);
    check_csp0_eq(&env, " ⊓ { a → STOP , SKIP }", expected);
    // missing `{`
    check_csp0_invalid("⊓");
    // missing process after `{`
    check_csp0_invalid("⊓ {");
    // missing `}`
    check_csp0_invalid("⊓ { STOP");
    // missing process after `,`
    check_csp0_invalid("⊓ { STOP,");
    check_csp0_invalid("⊓ { STOP, }");
    // a is undefined
    check_csp0_invalid("⊓ { a, STOP }");
    check_csp0_invalid("⊓ { STOP, a }");
}

TEST_CASE("parse: a → SKIP ; STOP")
{
    Environment env;
    auto expected = env.sequential_composition(
            env.prefix(Event("a"), env.skip()), env.stop());
    check_csp0_eq(&env, "a→SKIP;STOP", expected);
    check_csp0_eq(&env, " a→SKIP;STOP", expected);
    check_csp0_eq(&env, " a →SKIP;STOP", expected);
    check_csp0_eq(&env, " a → SKIP;STOP", expected);
    check_csp0_eq(&env, " a → SKIP ;STOP", expected);
    check_csp0_eq(&env, " a → SKIP ; STOP", expected);
    check_csp0_eq(&env, " a → SKIP ; STOP ", expected);
    // Fail to parse a bunch of invalid statements.
    // a is undefined
    check_csp0_invalid("a ; STOP");
    check_csp0_invalid("STOP ; a");
    // Missing process after ;
    check_csp0_invalid("SKIP;");
    check_csp0_invalid("SKIP ;");
    check_csp0_invalid("SKIP ; ");
}

TEST_CASE("associativity: a → SKIP ; b → SKIP ; c → SKIP")
{
    Environment env;
    auto expected = env.sequential_composition(
            env.prefix(Event("a"), env.skip()),
            env.sequential_composition(env.prefix(Event("b"), env.skip()),
                                       env.prefix(Event("c"), env.skip())));
    check_csp0_eq(&env, "a → SKIP ; b → SKIP ; c → SKIP", expected);
}

TEST_CASE("precedence: a → STOP □ b → STOP ⊓ c → STOP")
{
    Environment env;
    auto expected = env.internal_choice(
            env.external_choice(env.prefix(Event("a"), env.stop()),
                                env.prefix(Event("b"), env.stop())),
            env.prefix(Event("c"), env.stop()));
    // Expected result is
    // (a → STOP □ b → STOP) ⊓ (c → STOP)
    check_csp0_eq(&env, "a → STOP □ b → STOP ⊓ c → STOP", expected);
}

TEST_CASE("precedence: a → STOP □ b → SKIP ; c → STOP")
{
    Environment env;
    auto expected = env.external_choice(
            env.prefix(Event("a"), env.stop()),
            env.sequential_composition(env.prefix(Event("b"), env.skip()),
                                       env.prefix(Event("c"), env.stop())));
    // Expected result is
    // a → STOP □ (b → SKIP ; c → STOP)
    check_csp0_eq(&env, "a → STOP □ b → SKIP ; c → STOP", expected);
}

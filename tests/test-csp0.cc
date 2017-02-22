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

#include "hst/event.h"
#include "hst/external-choice.h"
#include "hst/internal-choice.h"
#include "hst/prefix.h"
#include "hst/process.h"
#include "hst/sequential-composition.h"
#include "hst/stop.h"

// Don't check the semantics of any of the operators here; this file just checks
// that the CSP₀ parser produces the same processes that you'd get constructing
// things by hand.  Look in test-operators.cc for test cases that verify that
// each operator behaves as we expect it to.

using hst::Event;
using hst::ExternalChoice;
using hst::InternalChoice;
using hst::ParseError;
using hst::Prefix;
using hst::Process;
using hst::SequentialComposition;
using hst::Skip;
using hst::Stop;

static void
check_csp0_valid(const std::string& csp0)
{
    ParseError error;
    if (!hst::load_csp0_string(csp0, &error)) {
        fail() << "Could not parse " << csp0 << ": " << error << abort_test();
    }
}

static void
check_csp0_invalid(const std::string& csp0)
{
    ParseError error;
    if (hst::load_csp0_string(csp0, &error)) {
        fail() << "Shouldn't be able to parse " << csp0 << abort_test();
    }
}

static void
check_csp0_eq(const std::string& csp0, const std::shared_ptr<Process>& expected)
{
    ParseError error;
    std::shared_ptr<Process> actual = hst::load_csp0_string(csp0, &error);
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
    auto expected = Stop::create();
    check_csp0_eq("STOP", expected);
    check_csp0_eq(" STOP", expected);
    check_csp0_eq("STOP ", expected);
    check_csp0_eq(" STOP ", expected);
}

TEST_CASE("parse: SKIP")
{
    auto expected = Skip::create();
    check_csp0_eq("SKIP", expected);
    check_csp0_eq(" SKIP", expected);
    check_csp0_eq("SKIP ", expected);
    check_csp0_eq(" SKIP ", expected);
}

TEST_CASE_GROUP("CSP₀ operators");

TEST_CASE("parse: a → STOP □ SKIP")
{
    auto expected = ExternalChoice::create(
            Prefix::create(Event("a"), Stop::create()), Skip::create());
    check_csp0_eq("a->STOP[]SKIP", expected);
    check_csp0_eq(" a->STOP[]SKIP", expected);
    check_csp0_eq(" a ->STOP[]SKIP", expected);
    check_csp0_eq(" a -> STOP[]SKIP", expected);
    check_csp0_eq(" a -> STOP []SKIP", expected);
    check_csp0_eq(" a -> STOP [] SKIP", expected);
    check_csp0_eq(" a -> STOP [] SKIP ", expected);
    check_csp0_eq("a→STOP□SKIP", expected);
    check_csp0_eq(" a→STOP□SKIP", expected);
    check_csp0_eq(" a →STOP□SKIP", expected);
    check_csp0_eq(" a → STOP□SKIP", expected);
    check_csp0_eq(" a → STOP □SKIP", expected);
    check_csp0_eq(" a → STOP □ SKIP", expected);
    check_csp0_eq(" a → STOP □ SKIP ", expected);
    // Fail to parse a bunch of invalid statements.
    // a is undefined
    check_csp0_invalid("a □ STOP");
    check_csp0_invalid("STOP □ a");
}

TEST_CASE("associativity: a → STOP □ b → STOP □ c → STOP")
{
    auto expected = ExternalChoice::create(
            Prefix::create(Event("a"), Stop::create()),
            ExternalChoice::create(Prefix::create(Event("b"), Stop::create()),
                                   Prefix::create(Event("c"), Stop::create())));
    check_csp0_eq("a -> STOP [] b -> STOP [] c -> STOP", expected);
    check_csp0_eq("a → STOP □ b → STOP □ c → STOP", expected);
}

TEST_CASE("parse: a → STOP ⊓ SKIP")
{
    auto expected = InternalChoice::create(
            Prefix::create(Event("a"), Stop::create()), Skip::create());
    check_csp0_eq("a->STOP|~|SKIP", expected);
    check_csp0_eq(" a->STOP|~|SKIP", expected);
    check_csp0_eq(" a ->STOP|~|SKIP", expected);
    check_csp0_eq(" a -> STOP|~|SKIP", expected);
    check_csp0_eq(" a -> STOP |~|SKIP", expected);
    check_csp0_eq(" a -> STOP |~| SKIP", expected);
    check_csp0_eq(" a -> STOP |~| SKIP ", expected);
    check_csp0_eq("a→STOP⊓SKIP", expected);
    check_csp0_eq(" a→STOP⊓SKIP", expected);
    check_csp0_eq(" a →STOP⊓SKIP", expected);
    check_csp0_eq(" a → STOP⊓SKIP", expected);
    check_csp0_eq(" a → STOP ⊓SKIP", expected);
    check_csp0_eq(" a → STOP ⊓ SKIP", expected);
    check_csp0_eq(" a → STOP ⊓ SKIP ", expected);
    // Fail to parse a bunch of invalid statements.
    // a is undefined
    check_csp0_invalid("a ⊓ STOP");
    check_csp0_invalid("STOP ⊓ a");
}

TEST_CASE("associativity: a → STOP ⊓ b → STOP ⊓ c → STOP")
{
    auto expected = InternalChoice::create(
            Prefix::create(Event("a"), Stop::create()),
            InternalChoice::create(Prefix::create(Event("b"), Stop::create()),
                                   Prefix::create(Event("c"), Stop::create())));
    check_csp0_eq("a -> STOP |~| b -> STOP |~| c -> STOP", expected);
    check_csp0_eq("a → STOP ⊓ b → STOP ⊓ c → STOP", expected);
}

TEST_CASE("parse: (STOP)")
{
    auto expected = Stop::create();
    check_csp0_eq("(STOP)", expected);
    check_csp0_eq(" (STOP)", expected);
    check_csp0_eq(" ( STOP)", expected);
    check_csp0_eq(" ( STOP )", expected);
    check_csp0_eq(" ( STOP ) ", expected);
    check_csp0_eq("((STOP))", expected);
    check_csp0_eq("(((STOP)))", expected);
}

TEST_CASE("parse: a → STOP")
{
    auto expected = Prefix::create(Event("a"), Stop::create());
    check_csp0_eq("a->STOP", expected);
    check_csp0_eq(" a->STOP", expected);
    check_csp0_eq(" a ->STOP", expected);
    check_csp0_eq(" a -> STOP", expected);
    check_csp0_eq(" a -> STOP ", expected);
    check_csp0_eq("a→STOP", expected);
    check_csp0_eq(" a→STOP", expected);
    check_csp0_eq(" a →STOP", expected);
    check_csp0_eq(" a → STOP", expected);
    check_csp0_eq(" a → STOP ", expected);
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
    auto expected = Prefix::create(Event("a"),
                                   Prefix::create(Event("b"), Stop::create()));
    check_csp0_eq("a -> b -> STOP", expected);
    check_csp0_eq("a → b → STOP", expected);
}

TEST_CASE("parse: □ {a → STOP, SKIP}")
{
    auto expected = ExternalChoice::create(
            Prefix::create(Event("a"), Stop::create()), Skip::create());
    check_csp0_eq("[]{a->STOP,SKIP}", expected);
    check_csp0_eq(" []{a->STOP,SKIP}", expected);
    check_csp0_eq(" [] {a->STOP,SKIP}", expected);
    check_csp0_eq(" [] { a->STOP,SKIP}", expected);
    check_csp0_eq(" [] { a ->STOP,SKIP}", expected);
    check_csp0_eq(" [] { a -> STOP,SKIP}", expected);
    check_csp0_eq(" [] { a -> STOP ,SKIP}", expected);
    check_csp0_eq(" [] { a -> STOP , SKIP}", expected);
    check_csp0_eq(" [] { a -> STOP , SKIP }", expected);
    check_csp0_eq(" [] { a -> STOP , SKIP } ", expected);
    check_csp0_eq("□{a→STOP,SKIP}", expected);
    check_csp0_eq(" □{a→STOP,SKIP}", expected);
    check_csp0_eq(" □ {a→STOP,SKIP}", expected);
    check_csp0_eq(" □ { a→STOP,SKIP}", expected);
    check_csp0_eq(" □ { a →STOP,SKIP}", expected);
    check_csp0_eq(" □ { a → STOP,SKIP}", expected);
    check_csp0_eq(" □ { a → STOP ,SKIP}", expected);
    check_csp0_eq(" □ { a → STOP , SKIP}", expected);
    check_csp0_eq(" □ { a → STOP , SKIP }", expected);
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

TEST_CASE("parse: ⊓ {a → STOP, SKIP}")
{
    auto expected = InternalChoice::create(
            Prefix::create(Event("a"), Stop::create()), Skip::create());
    check_csp0_eq("|~|{a->STOP,SKIP}", expected);
    check_csp0_eq(" |~|{a->STOP,SKIP}", expected);
    check_csp0_eq(" |~| {a->STOP,SKIP}", expected);
    check_csp0_eq(" |~| { a->STOP,SKIP}", expected);
    check_csp0_eq(" |~| { a ->STOP,SKIP}", expected);
    check_csp0_eq(" |~| { a -> STOP,SKIP}", expected);
    check_csp0_eq(" |~| { a -> STOP ,SKIP}", expected);
    check_csp0_eq(" |~| { a -> STOP , SKIP}", expected);
    check_csp0_eq(" |~| { a -> STOP , SKIP }", expected);
    check_csp0_eq(" |~| { a -> STOP , SKIP } ", expected);
    check_csp0_eq("⊓{a→STOP,SKIP}", expected);
    check_csp0_eq(" ⊓{a→STOP,SKIP}", expected);
    check_csp0_eq(" ⊓ {a→STOP,SKIP}", expected);
    check_csp0_eq(" ⊓ { a→STOP,SKIP}", expected);
    check_csp0_eq(" ⊓ { a →STOP,SKIP}", expected);
    check_csp0_eq(" ⊓ { a → STOP,SKIP}", expected);
    check_csp0_eq(" ⊓ { a → STOP ,SKIP}", expected);
    check_csp0_eq(" ⊓ { a → STOP , SKIP}", expected);
    check_csp0_eq(" ⊓ { a → STOP , SKIP }", expected);
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
    auto expected = SequentialComposition::create(
            Prefix::create(Event("a"), Skip::create()), Stop::create());
    check_csp0_eq("a→SKIP;STOP", expected);
    check_csp0_eq(" a→SKIP;STOP", expected);
    check_csp0_eq(" a →SKIP;STOP", expected);
    check_csp0_eq(" a → SKIP;STOP", expected);
    check_csp0_eq(" a → SKIP ;STOP", expected);
    check_csp0_eq(" a → SKIP ; STOP", expected);
    check_csp0_eq(" a → SKIP ; STOP ", expected);
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
    auto expected = SequentialComposition::create(
            Prefix::create(Event("a"), Skip::create()),
            SequentialComposition::create(
                    Prefix::create(Event("b"), Skip::create()),
                    Prefix::create(Event("c"), Skip::create())));
    check_csp0_eq("a → SKIP ; b → SKIP ; c → SKIP", expected);
}

TEST_CASE("precedence: a → STOP □ b → STOP ⊓ c → STOP")
{
    auto expected = InternalChoice::create(
            ExternalChoice::create(Prefix::create(Event("a"), Stop::create()),
                                   Prefix::create(Event("b"), Stop::create())),
            Prefix::create(Event("c"), Stop::create()));
    // Expected result is
    // (a → STOP □ b → STOP) ⊓ (c → STOP)
    check_csp0_eq("a → STOP □ b → STOP ⊓ c → STOP", expected);
}

TEST_CASE("precedence: a → STOP □ b → SKIP ; c → STOP")
{
    auto expected = ExternalChoice::create(
            Prefix::create(Event("a"), Stop::create()),
            SequentialComposition::create(
                    Prefix::create(Event("b"), Skip::create()),
                    Prefix::create(Event("c"), Stop::create())));
    // Expected result is
    // a → STOP □ (b → SKIP ; c → STOP)
    check_csp0_eq("a → STOP □ b → SKIP ; c → STOP", expected);
}

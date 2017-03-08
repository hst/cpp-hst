/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <string>

#include "test-cases.h"
#include "test-harness.cc.in"

#include "hst/csp0.h"
#include "hst/environment.h"
#include "hst/event.h"
#include "hst/process.h"
#include "hst/refinement.h"
#include "hst/semantic-models.h"

using hst::Environment;
using hst::Event;
using hst::NormalizedProcess;
using hst::ParseError;
using hst::Process;
using hst::RefinementChecker;
using hst::Traces;

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

template <typename Model>
void
check_refinement(const std::string& spec_csp0, const std::string& impl_csp0)
{
    Environment env;
    const Process* spec = require_csp0(&env, spec_csp0);
    const NormalizedProcess* prenormalized_spec = env.prenormalize(spec);
    const NormalizedProcess* normalized_spec =
            env.normalize<Model>(prenormalized_spec);
    const Process* impl = require_csp0(&env, impl_csp0);
    RefinementChecker<Model> checker;
    if (!checker.refines(normalized_spec, impl)) {
        fail() << "Expected refinement to hold: " << spec_csp0 << " ⊑"
               << Model::abbreviation() << " " << impl_csp0 << abort_test();
    }
}

template <typename Model>
void
xcheck_refinement(const std::string& spec_csp0, const std::string& impl_csp0)
{
    Environment env;
    const Process* spec = require_csp0(&env, spec_csp0);
    const NormalizedProcess* prenormalized_spec = env.prenormalize(spec);
    const NormalizedProcess* normalized_spec =
            env.normalize<Model>(prenormalized_spec);
    const Process* impl = require_csp0(&env, impl_csp0);
    RefinementChecker<Model> checker;
    if (checker.refines(normalized_spec, impl)) {
        fail() << "Expected refinement to NOT hold: " << spec_csp0 << " ⊑"
               << Model::abbreviation() << " " << impl_csp0 << abort_test();
    }
}

}  // namespace

TEST_CASE_GROUP("traces refinement");

TEST_CASE("STOP")
{
    check_refinement<Traces>("STOP", "STOP");
    xcheck_refinement<Traces>("STOP", "a → STOP");
    xcheck_refinement<Traces>("STOP", "a → STOP □ b → STOP");
    xcheck_refinement<Traces>("STOP", "a → STOP ⊓ b → STOP");
}

TEST_CASE("a → STOP")
{
    check_refinement<Traces>("a → STOP", "STOP");
    check_refinement<Traces>("a → STOP", "a → STOP");
    xcheck_refinement<Traces>("a → STOP", "a → STOP □ b → STOP");
    xcheck_refinement<Traces>("a → STOP", "a → STOP ⊓ b → STOP");
}

TEST_CASE("a → STOP □ b → STOP")
{
    check_refinement<Traces>("a → STOP □ b → STOP", "STOP");
    check_refinement<Traces>("a → STOP □ b → STOP", "a → STOP");
    check_refinement<Traces>("a → STOP □ b → STOP", "a → STOP □ b → STOP");
    check_refinement<Traces>("a → STOP □ b → STOP", "a → STOP ⊓ b → STOP");
}

TEST_CASE("a → STOP ⊓ b → STOP")
{
    check_refinement<Traces>("a → STOP ⊓ b → STOP", "STOP");
    check_refinement<Traces>("a → STOP ⊓ b → STOP", "a → STOP");
    check_refinement<Traces>("a → STOP ⊓ b → STOP", "a → STOP □ b → STOP");
    check_refinement<Traces>("a → STOP ⊓ b → STOP", "a → STOP ⊓ b → STOP");
}

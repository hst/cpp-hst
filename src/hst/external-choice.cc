/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/environment.h"

#include <memory>
#include <ostream>

#include "hst/event.h"
#include "hst/hash.h"
#include "hst/process.h"

namespace hst {

namespace {

class ExternalChoice : public Process {
  public:
    ExternalChoice(Environment* env, Process::Set ps)
        : env_(env), ps_(std::move(ps))
    {
    }

    void initials(Event::Set* out) override;
    void afters(Event initial, Process::Set* out) override;

    std::size_t hash() const override;
    bool operator==(const Process& other) const override;
    unsigned int precedence() const override { return 6; }
    void print(std::ostream& out) const override;

  private:
    Environment* env_;
    Process::Set ps_;
};

}  // namespace

Process*
Environment::external_choice(Process::Set ps)
{
    return register_process(
            std::unique_ptr<Process>(new ExternalChoice(this, std::move(ps))));
}

Process*
Environment::external_choice(Process* p, Process* q)
{
    return external_choice(Process::Set{p, q});
}

// Operational semantics for □ Ps
//
//                  P -τ→ P'
//  1)  ────────────────────────────── P ∈ Ps
//       □ Ps -τ→ □ (Ps ∖ {P} ∪ {P'})
//
//         P -a→ P'
//  2)  ───────────── P ∈ Ps, a ≠ τ
//       □ Ps -a→ P'

void
ExternalChoice::initials(Event::Set* out)
{
    // 1) If P ∈ Ps can perform τ, then □ Ps can perform τ.
    // 2) If P ∈ Ps can perform a ≠ τ, then □ Ps can perform a ≠ τ.
    //
    // initials(□ Ps) = ⋃ { initials(P) ∩ {τ} | P ∈ Ps }                [rule 1]
    //                ∪ ⋃ { initials(P) ∖ {τ} | P ∈ Ps }                [rule 2]
    //
    //                = ⋃ { initials(P) | P ∈ Ps }
    for (const auto& p : ps_) {
        p->initials(out);
    }
}

void
ExternalChoice::afters(Event initial, Process::Set* out)
{
    // afters(□ Ps, τ) = ⋃ { □ Ps ∖ {P} ∪ {P'} | P ∈ Ps, P' ∈ afters(P, τ) }
    //                                                                  [rule 1]
    // afters(□ Ps, a ≠ τ) = ⋃ { P' | P ∈ Ps, P' ∈ afters(P, a) }       [rule 2]
    if (initial == Event::tau()) {
        // We're going to build up a lot of new Ps' sets that all have the same
        // basic structure: Ps' = Ps ∖ {P} ∪ {P'}.  Each Ps' starts with Ps, so
        // go ahead and add that into our Ps' set once.
        Process::Set ps_prime(ps_);
        for (const auto& p : ps_) {
            // Set Ps' to Ps ∖ {P}
            ps_prime.erase(p);
            // Grab afters(P, τ)
            Process::Set p_afters;
            p->afters(initial, &p_afters);
            for (const auto& p_prime : p_afters) {
                // ps_prime currently contains (Ps ∖ {P}).  Add P' to produce
                // (Ps ∖ {P} ∪ {P'})
                ps_prime.insert(p_prime);
                // Create □ (Ps ∖ {P} ∪ {P'}) as a result.
                out->insert(env_->external_choice(ps_prime));
                // Reset Ps' back to Ps ∖ {P}.
                ps_prime.erase(p_prime);
            }
            // Reset Ps' back to Ps.
            ps_prime.insert(p);
        }
    } else {
        for (const auto& p : ps_) {
            p->afters(initial, out);
        }
    }
}

std::size_t
ExternalChoice::hash() const
{
    static hash_scope external_choice;
    return hasher(external_choice).add(ps_).value();
}

bool
ExternalChoice::operator==(const Process& other_) const
{
    const ExternalChoice* other = dynamic_cast<const ExternalChoice*>(&other_);
    if (other == nullptr) {
        return false;
    }
    return ps_ == other->ps_;
}

void
ExternalChoice::print(std::ostream& out) const
{
    print_subprocesses(out, ps_, "□");
}

}  // namespace hst

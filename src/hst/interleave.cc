/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/environment.h"

#include <string>

#include "hst/event.h"
#include "hst/hash.h"
#include "hst/process.h"

namespace hst {

namespace {

class Interleave : public Process {
  public:
    Interleave(Environment* env, Process::Bag ps)
        : env_(env), ps_(std::move(ps))
    {
    }

    void initials(Event::Set* out) const override;
    void afters(Event initial, Process::Set* out) const override;

    std::size_t hash() const override;
    bool operator==(const Process& other) const override;
    unsigned int precedence() const override { return 7; }
    void print(std::ostream& out) const override;

  private:
    void normal_afters(Event initial, Process::Set* out) const;
    void tau_afters(Event initial, Process::Set* out) const;
    void tick_afters(Event initial, Process::Set* out) const;

    Environment* env_;
    Process::Bag ps_;
};

}  // namespace

const Process*
Environment::interleave(Process::Bag ps)
{
    return register_process(
            std::unique_ptr<Process>(new Interleave(this, std::move(ps))));
}

const Process*
Environment::interleave(const Process* p, const Process* q)
{
    return interleave(Process::Bag{p, q});
}

// Operational semantics for ⫴ Ps
//
//                  P -τ→ P'
//  1)  ────────────────────────────── P ∈ Ps
//       ⫴ Ps -τ→ ⫴ (Ps ∖ {P} ∪ {P'})
//
//                  P -a→ P'
//  2)  ────────────────────────────── P ∈ Ps, a ∉ {τ,✔}
//       ⫴ Ps -a→ ⫴ (Ps ∖ {P} ∪ {P'})
//
//                  P -✔→ P'
//  3)  ──────────────────────────────── P ∈ Ps
//       ⫴ Ps -τ→ ⫴ (Ps ∖ {P} ∪ {STOP})
//
//  4)  ───────────────────
//       ⫴ {STOP} -✔→ STOP

void
Interleave::initials(Event::Set* out) const
{
    // initials(⫴ Ps) = ⋃ { initials(P) ∩ {τ} | P ∈ Ps }                [rule 1]
    //                ∪ ⋃ { initials(P) ∖ {τ,✔} | P ∈ Ps }              [rule 2]
    //                ∪ ⋃ { (✔ ∈ initials(P)? {τ}: {}) | P ∈ Ps }       [rule 3]
    //                ∪ (Ps = {STOP}? {✔}: {})                          [rule 4]

    // Rules 1 and 2
    for (const Process* process : ps_) {
        process->initials(out);
    }
    // Rule 3
    if (out->erase(Event::tick()) > 0) {
        out->insert(Event::tau());
    }
    // Rule 4
    if (out->empty()) {
        out->insert(Event::tick());
    }
}

void
Interleave::normal_afters(Event initial, Process::Set* out) const
{
    // afters(⫴ Ps, a ∉ {τ,✔}) = ⋃ { ⫴ Ps ∖ {P} ∪ {P'} |
    //                                  P ∈ Ps, P' ∈ afters(P, a) }     [rule 2]

    // We're going to build up a lot of new Ps' sets that all have the same
    // basic structure: Ps' = Ps ∖ {P} ∪ {P'}.  Each Ps' starts with Ps, so go
    // ahead and add that to our Ps' set once.
    Process::Bag ps_prime(ps_);
    for (const Process* p : ps_) {
        // Set Ps' to Ps ∖ {P}
        ps_prime.erase(ps_prime.find(p));
        // Grab afters(P, a)
        Process::Set p_afters;
        p->afters(initial, &p_afters);
        for (const Process* p_prime : p_afters) {
            // ps_prime currently contains Ps.  Add P' and remove P to produce
            // (Ps ∖ {P} ∪ {P'})
            ps_prime.insert(p_prime);
            // Create ⫴ (Ps ∖ {P} ∪ {P'}) as a result.
            out->insert(env_->interleave(ps_prime));
            // Reset Ps' back to Ps ∖ {P}.
            ps_prime.erase(ps_prime.find(p_prime));
        }
        // Reset Ps' back to Ps.
        ps_prime.insert(p);
    }
}

void
Interleave::tau_afters(Event initial, Process::Set* out) const
{
    // afters(⫴ Ps, τ) = ⋃ { ⫴ Ps ∖ {P} ∪ {P'} | P ∈ Ps, P' ∈ afters(P, τ) }
    //                                                                  [rule 1]
    //                 ∪ ⋃ { ⫴ Ps ∖ {P} ∪ {STOP} | P ∈ Ps, P' ∈ afters(P, ✔) }
    //                                                                  [rule 3]
    // Rule 1 has the same form as rule 2, which we've implemented above.*/
    normal_afters(initial, out);
    // Rule 3...does not.
    // We're going to build up a lot of new Ps' sets that all have the same
    // basic structure: Ps' = Ps ∖ {P} ∪ {STOP}.  Each Ps' starts with Ps, so go
    // ahead and add that to our Ps' set once.
    Process::Bag ps_prime(ps_);
    // Find each P ∈ Ps where ✔ ∈ initials(P).
    for (const Process* p : ps_) {
        Event::Set initials;
        p->initials(&initials);
        if (initials.find(Event::tick()) != initials.end()) {
            // Create Ps ∖ {P} ∪ {STOP}) as a result.
            ps_prime.erase(ps_prime.find(p));
            ps_prime.insert(env_->stop());
            out->insert(env_->interleave(ps_prime));
            // Reset Ps' back to Ps.
            ps_prime.erase(ps_prime.find(env_->stop()));
            ps_prime.insert(p);
        }
    }
}

void
Interleave::tick_afters(Event initial, Process::Set* out) const
{
    // afters(⫴ {STOP}, ✔) = {STOP}                                     [rule 4]
    for (const Process* p : ps_) {
        Event::Set initials;
        p->initials(&initials);
        if (!initials.empty()) {
            // One of the subprocesses has at least one initial, so this cannot
            // possibly be ⫴ {STOP}.
            return;
        }
    }
    out->insert(env_->stop());
}

void
Interleave::afters(Event initial, Process::Set* out) const
{
    if (initial == Event::tau()) {
        tau_afters(initial, out);
    } else if (initial == Event::tick()) {
        tick_afters(initial, out);
    } else {
        normal_afters(initial, out);
    }
}

std::size_t
Interleave::hash() const
{
    static hash_scope internal_choice;
    return hasher(internal_choice).add(ps_).value();
}

bool
Interleave::operator==(const Process& other_) const
{
    const Interleave* other = dynamic_cast<const Interleave*>(&other_);
    if (other == nullptr) {
        return false;
    }
    return ps_ == other->ps_;
}

void
Interleave::print(std::ostream& out) const
{
    print_subprocesses(out, ps_, "⫴");
}

}  // namespace hst

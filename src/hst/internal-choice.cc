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

class InternalChoice : public Process {
  public:
    explicit InternalChoice(Process::Set ps) : ps_(std::move(ps)) {}
    void initials(Event::Set* out) const override;
    void afters(Event initial, Process::Set* out) const override;

    std::size_t hash() const override;
    bool operator==(const Process& other) const override;
    unsigned int precedence() const override { return 7; }
    void print(std::ostream& out) const override;

  private:
    Process::Set ps_;
};

}  // namespace

const Process*
Environment::internal_choice(Process::Set ps)
{
    return register_process(new InternalChoice(std::move(ps)));
}

const Process*
Environment::internal_choice(const Process* p, const Process* q)
{
    return internal_choice(Process::Set{p, q});
}

// Operational semantics for ⊓ Ps
//
// 1) ──────────── P ∈ Ps
//     ⊓ Ps -τ→ P

void
InternalChoice::initials(Event::Set* out) const
{
    // initials(⊓ Ps) = {τ}
    out->insert(Event::tau());
}

void
InternalChoice::afters(Event initial, Process::Set* out) const
{
    // afters(⊓ Ps, τ) = Ps
    if (initial == Event::tau()) {
        out->insert(ps_.begin(), ps_.end());
    }
}

std::size_t
InternalChoice::hash() const
{
    static hash_scope internal_choice;
    return hasher(internal_choice).add(ps_).value();
}

bool
InternalChoice::operator==(const Process& other_) const
{
    const InternalChoice* other = dynamic_cast<const InternalChoice*>(&other_);
    if (other == nullptr) {
        return false;
    }
    return ps_ == other->ps_;
}

void
InternalChoice::print(std::ostream& out) const
{
    print_subprocesses(out, ps_, "⊓");
}

}  // namespace hst

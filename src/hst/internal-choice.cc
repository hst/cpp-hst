/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/internal-choice.h"

#include <string>

#include "hst/event.h"
#include "hst/hash.h"
#include "hst/process.h"

namespace hst {

class ConcreteInternalChoice : public InternalChoice {
  public:
    ConcreteInternalChoice(Process::Set ps) : InternalChoice(std::move(ps)) {}
};

std::shared_ptr<InternalChoice>
InternalChoice::create(Process::Set ps)
{
    return std::make_shared<ConcreteInternalChoice>(ps);
}

std::shared_ptr<InternalChoice>
InternalChoice::create(std::shared_ptr<Process> p, std::shared_ptr<Process> q)
{
    return std::make_shared<ConcreteInternalChoice>(
            Process::Set{std::move(p), std::move(q)});
}

// Operational semantics for ⊓ Ps
//
// 1) ──────────── P ∈ Ps
//     ⊓ Ps -τ→ P

void
InternalChoice::initials(Event::Set* out)
{
    // initials(⊓ Ps) = {τ}
    out->insert(Event::tau());
}

void
InternalChoice::afters(Event initial, Process::Set* out)
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
    print_subprocess_set(out, ps_, "⊓");
}

}  // namespace hst

/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/operators.h"

#include <memory>
#include <ostream>

#include "hst/event.h"
#include "hst/hash.h"
#include "hst/process.h"

namespace hst {

class InternalChoice : public Process {
  public:
    explicit InternalChoice(Process::Set ps) : ps_(std::move(ps)) {}
    void initials(Event::Set* out) override;
    void afters(Event initial, Process::Set* out) override;

    std::size_t hash() const override;
    bool operator==(const Process& other) const override;
    unsigned int precedence() const override { return 7; }
    void print(std::ostream& out) const override;

  private:
    Process::Set ps_;
};

std::shared_ptr<Process>
internal_choice(Process::Set ps)
{
    return std::make_shared<InternalChoice>(ps);
}

std::shared_ptr<Process>
internal_choice(std::shared_ptr<Process> p, std::shared_ptr<Process> q)
{
    return std::make_shared<InternalChoice>(
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

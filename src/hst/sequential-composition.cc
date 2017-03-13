/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/environment.h"

#include <functional>
#include <memory>
#include <ostream>

#include "hst/event.h"
#include "hst/hash.h"
#include "hst/process.h"

namespace hst {

namespace {

class SequentialComposition : public Process {
  public:
    SequentialComposition(Environment* env, const Process* p, const Process* q)
        : env_(env), p_(p), q_(q)
    {
    }

    void initials(std::function<void(Event)> op) const override;
    void afters(Event initial,
                std::function<void(const Process&)> op) const override;
    void subprocesses(std::function<void(const Process&)> op) const override;

    std::size_t hash() const override;
    bool operator==(const Process& other) const override;
    unsigned int precedence() const override { return 3; }
    void print(std::ostream& out) const override;

  private:
    Environment* env_;
    const Process* p_;
    const Process* q_;
};

}  // namespace

const Process*
Environment::sequential_composition(const Process* p, const Process* q)
{
    return register_process(new SequentialComposition(this, p, q));
}

// Operational semantics for P ; Q
//
//        P -a→ P'
// 1)  ────────────── a ≠ ✔
//      P;Q -a→ P';Q
//
//     ∃ P' • P -✔→ P'
// 2) ─────────────────
//       P;Q -τ→ Q

void
SequentialComposition::initials(std::function<void(Event)> op) const
{
    // 1) P;Q can perform all of the same events as P, except for ✔.
    // 2) If P can perform ✔, then P;Q can perform τ.
    //
    // initials(P;Q) = initials(P) ∖ {✔}                                [rule 1]
    //               ∪ (✔ ∈ initials(P)? {τ}: {})                       [rule 2]
    p_->initials([&op](Event initial) {
        if (initial == Event::tick()) {
            op(Event::tau());
        } else {
            op(initial);
        }
    });
}

void
SequentialComposition::afters(Event initial,
                              std::function<void(const Process&)> op) const
{
    // afters(P;Q a ≠ ✔) = afters(P, a)                                 [rule 1]
    // afters(P;Q, τ) = Q  if ✔ ∈ initials(P)                           [rule 2]
    //                = {} if ✔ ∉ initials(P)
    // afters(P;Q, ✔) = {}
    //
    // (Note that τ is covered by both rules)

    // The composition can never perform a ✔; that's always translated into a τ
    // that activates process Q.
    if (initial == Event::tick()) {
        return;
    }

    // If P can perform a non-✔ event (including τ) leading to P', then P;Q can
    // also perform that event, leading to P';Q.
    p_->afters(initial, [this, &op](const Process& p_prime) {
        op(*env_->sequential_composition(&p_prime, q_));
    });

    // If P can perform a ✔ leading to P', then P;Q can perform a τ leading to
    // Q.  Note that we don't care what P' is; we just care that it exists.
    if (initial == Event::tau()) {
        bool any_ticks = false;
        Process::Set afters;
        p_->afters(Event::tick(),
                   [&any_ticks](const Process& _) { any_ticks = true; });
        if (any_ticks) {
            // P can perform ✔, and we don't actually care what it leads to,
            // since we're going to lead to Q no matter what.
            op(*q_);
        }
    }
}

void
SequentialComposition::subprocesses(
        std::function<void(const Process&)> op) const
{
    op(*p_);
    op(*q_);
}

std::size_t
SequentialComposition::hash() const
{
    static hash_scope sequential_composition;
    return hasher(sequential_composition).add(*p_).add(*q_).value();
}

bool
SequentialComposition::operator==(const Process& other_) const
{
    const SequentialComposition* other =
            dynamic_cast<const SequentialComposition*>(&other_);
    if (other == nullptr) {
        return false;
    }
    return *p_ == *other->p_ && *q_ == *other->q_;
}

void
SequentialComposition::print(std::ostream& out) const
{
    print_subprocess(out, *p_);
    out << " ; ";
    print_subprocess(out, *q_);
}

}  // namespace hst

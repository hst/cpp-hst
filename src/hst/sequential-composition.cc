/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/sequential-composition.h"

#include <ostream>
#include <set>

#include "hst/event.h"
#include "hst/hash.h"
#include "hst/process.h"
#include "hst/stop.h"

namespace hst {

class ConcreteSkip : public Skip {};

std::shared_ptr<Skip>
Skip::create()
{
    static std::shared_ptr<Skip> stop = std::make_shared<ConcreteSkip>();
    return stop;
}

void
Skip::initials(Event::Set* out)
{
    out->insert(Event::tick());
}

void
Skip::afters(Event initial, Process::Set* out)
{
    if (initial == Event::tick()) {
        out->insert(Stop::create());
    }
}

std::size_t
Skip::hash() const
{
    static hash_scope skip;
    return hasher(skip).value();
}

bool
Skip::operator==(const Process& other_) const
{
    const Skip* other = dynamic_cast<const Skip*>(&other_);
    if (other == nullptr) {
        return false;
    }
    return true;
}

void
Skip::print(std::ostream& out) const
{
    out << "SKIP";
}

class ConcreteSequentialComposition : public SequentialComposition {
  public:
    ConcreteSequentialComposition(std::shared_ptr<Process> p,
                                  std::shared_ptr<Process> q)
        : SequentialComposition(std::move(p), std::move(q))
    {
    }
};

std::shared_ptr<SequentialComposition>
SequentialComposition::create(std::shared_ptr<Process> p,
                              std::shared_ptr<Process> q)
{
    return std::make_shared<ConcreteSequentialComposition>(std::move(p),
                                                           std::move(q));
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
SequentialComposition::initials(std::set<Event>* out)
{
    // 1) P;Q can perform all of the same events as P, except for ✔.
    // 2) If P can perform ✔, then P;Q can perform τ.
    //
    // initials(P;Q) = initials(P) ∖ {✔}                                [rule 1]
    //               ∪ (✔ ∈ initials(P)? {τ}: {})                       [rule 2]
    p_->initials(out);
    if (out->erase(Event::tick())) {
        out->insert(Event::tau());
    }
}

void
SequentialComposition::afters(Event initial, Process::Set* out)
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
    {
        Process::Set afters;
        p_->afters(initial, &afters);
        for (const auto& p_prime : afters) {
            out->insert(SequentialComposition::create(p_prime, q_));
        }
    }

    // If P can perform a ✔ leading to P', then P;Q can perform a τ leading to
    // Q.  Note that we don't care what P' is; we just care that it exists.
    if (initial == Event::tau()) {
        Process::Set afters;
        p_->afters(Event::tick(), &afters);
        if (!afters.empty()) {
            // P can perform ✔, and we don't actually care what it leads to,
            // since we're going to lead to Q no matter what.
            out->insert(q_);
        }
    }
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

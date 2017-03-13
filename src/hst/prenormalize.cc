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

class Prenormalization : public NormalizedProcess {
  public:
    Prenormalization(Environment* env, Process::Set ps) : env_(env), ps_(ps)
    {
        ps_.tau_close();
    }

    void initials(std::function<void(Event)> op) const override;
    const NormalizedProcess* after(Event initial) const override;
    void subprocesses(std::function<void(const Process&)> op) const override;
    void expand(std::function<void(const Process&)> op) const override;

    std::size_t hash() const override;
    bool operator==(const Process& other) const override;
    unsigned int precedence() const override { return 0; }
    void print(std::ostream& out) const override;

  private:
    Environment* env_;
    Process::Set ps_;  // constructor ensures this is τ-closed
};

}  // namespace

const NormalizedProcess*
Environment::prenormalize(Process::Set ps)
{
    return register_process(new Prenormalization(this, std::move(ps)));
}

const NormalizedProcess*
Environment::prenormalize(const Process* p)
{
    return prenormalize(Process::Set{p});
}

void
Prenormalization::initials(std::function<void(Event)> op) const
{
    // Find all of the non-τ events that any of the underlying processes can
    // perform.
    for (const Process* p : ps_) {
        p->initials([&op](Event initial) {
            if (initial != Event::tau()) {
                op(initial);
            }
        });
    }
}

const NormalizedProcess*
Prenormalization::after(Event initial) const
{
    // Prenormalized processes can never perform a τ.
    if (initial == Event::tau()) {
        return nullptr;
    }

    // Find the set of processes that you could end up in by starting in one of
    // our underlying processes and following a single `initial` event.
    Process::Set afters;
    for (const Process* p : ps_) {
        p->afters(initial, [&afters](const Process& process) {
            afters.insert(&process);
        });
    }

    // Since a normalized process can only have one `after` for any event, merge
    // together all of the possible afters into a single prenormalized process.
    return env_->prenormalize(std::move(afters));
}

void
Prenormalization::subprocesses(std::function<void(const Process&)> op) const
{
    for (const Process* process : ps_) {
        op(*process);
    }
}

void
Prenormalization::expand(std::function<void(const Process&)> op) const
{
    for (const Process* process : ps_) {
        op(*process);
    }
}

std::size_t
Prenormalization::hash() const
{
    static hash_scope prenormalized;
    return hasher(prenormalized).add(ps_).value();
}

bool
Prenormalization::operator==(const Process& other_) const
{
    const Prenormalization* other =
            dynamic_cast<const Prenormalization*>(&other_);
    if (other == nullptr) {
        return false;
    }
    return ps_ == other->ps_;
}

void
Prenormalization::print(std::ostream& out) const
{
    out << "prenormalize " << ps_;
}

}  // namespace hst

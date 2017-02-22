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

class Stop : public Process {
  public:
    Stop() = default;

    void initials(Event::Set* out) override;
    void afters(Event initial, Process::Set* out) override;

    std::size_t hash() const override;
    bool operator==(const Process& other) const override;
    unsigned int precedence() const override { return 1; }
    void print(std::ostream& out) const override;
};

std::shared_ptr<Process>
stop()
{
    static std::shared_ptr<Process> stop = std::make_shared<Stop>();
    return stop;
}

void
Stop::initials(Event::Set* out)
{
}

void
Stop::afters(Event initial, Process::Set* out)
{
}

std::size_t
Stop::hash() const
{
    static hash_scope stop;
    return hasher(stop).value();
}

bool
Stop::operator==(const Process& other_) const
{
    const Stop* other = dynamic_cast<const Stop*>(&other_);
    if (other == nullptr) {
        return false;
    }
    return true;
}

void
Stop::print(std::ostream& out) const
{
    out << "STOP";
}

}  // namespace hst

/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/stop.h"

#include <memory>

#include "hst/event.h"
#include "hst/hash.h"
#include "hst/process.h"

namespace hst {

class ConcreteStop : public Stop {};

std::shared_ptr<Stop>
Stop::create()
{
    static std::shared_ptr<Stop> stop = std::make_shared<ConcreteStop>();
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

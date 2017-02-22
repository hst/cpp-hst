/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/stop.h"

#include <memory>

#include "hst/event.h"
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

void
Stop::print(std::ostream& out) const
{
    out << "STOP";
}

}  // namespace hst

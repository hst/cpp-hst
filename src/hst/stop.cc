/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/stop.h"

#include "hst/event.h"
#include "hst/process.h"

namespace hst {

Stop*
Stop::get()
{
    static Stop stop;
    return &stop;
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

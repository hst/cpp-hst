/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/prefix.h"

#include <ostream>
#include <set>

#include "hst/event.h"
#include "hst/process.h"

namespace hst {

void
Prefix::initials(std::set<Event>* out)
{
    out->insert(a_);
}

void
Prefix::afters(Event initial, std::set<Process*>* out)
{
    if (initial == a_) {
        out->insert(p_);
    }
}

void
Prefix::print(std::ostream& out) const
{
    out << a_ << " → ";
    print_subprocess(out, *p_);
}

}  // namespace hst

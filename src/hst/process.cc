/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/process.h"

namespace hst {

bool
operator==(const Process::Set& lhs, const Process::Set& rhs)
{
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (const auto& process : lhs) {
        if (rhs.find(process) == rhs.end()) {
            return false;
        }
    }
    return true;
}

std::ostream& operator<<(std::ostream& out, const Process::Set& processes)
{
    bool first = true;
    out << "{";
    for (auto process : processes) {
        if (first) {
            first = false;
        } else {
            out << ", ";
        }
        out << *process;
    }
    return out << "}";
}

}  // namespace hst

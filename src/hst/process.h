/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_PROCESS_H
#define HST_PROCESS_H

#include <memory>
#include <ostream>
#include <set>

#include "hst/event.h"

namespace hst {

class Process {
  public:
    using Set = std::set<std::shared_ptr<Process>>;

    virtual ~Process() = default;

    // Fill `out` with the initial events of this process.
    virtual void initials(Event::Set* out) = 0;

    // Fill `out` with the subprocesses that you reach after following a single
    // `initial` event from this process.
    virtual void afters(Event initial, Set* out) = 0;

    virtual unsigned int precedence() const = 0;
    virtual void print(std::ostream& out) const = 0;

    // Prints a subprocess of this process.  The precedence values of the two
    // are compared to automatically decide whether we need to include
    // parentheses or not.
    void
    print_subprocess(std::ostream& out, const Process& inner) const
    {
        if (precedence() < inner.precedence()) {
            out << "(";
            inner.print(out);
            out << ")";
        } else {
            inner.print(out);
        }
    }
};

inline std::ostream&
operator<<(std::ostream& out, const Process& process)
{
    process.print(out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const Process::Set& processes);

}  // namespace hst
#endif  // HST_PROCESS_H

/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/process.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "hst/hash.h"

namespace hst {

void
NormalizedProcess::afters(Event initial, Set* out) const
{
    const NormalizedProcess* process = after(initial);
    if (process) {
        out->insert(process);
    }
}

std::size_t
Process::Bag::hash() const
{
    static hash_scope scope;
    hst::hasher hash(scope);
    for (const Process* process : *this) {
        hash.add_unordered(*process);
    }
    return hash.value();
}

bool
operator==(const Process::Bag& lhs, const Process::Bag& rhs)
{
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (const Process* process : lhs) {
        if (rhs.find(process) == rhs.end()) {
            return false;
        }
    }
    return true;
}

std::ostream& operator<<(std::ostream& out, const Process::Bag& processes)
{
    // We want reproducible output, so we sort the processes in the set before
    // rendering them into the stream.  We the process's index to print out the
    // processes in the order that they were defined.
    std::vector<const Process*> sorted_processes(processes.begin(),
                                                 processes.end());
    std::sort(sorted_processes.begin(), sorted_processes.end(),
              [](const Process* p1, const Process* p2) {
                  return p1->index() < p2->index();
              });

    bool first = true;
    out << "{";
    for (const Process* process : sorted_processes) {
        if (first) {
            first = false;
        } else {
            out << ", ";
        }
        out << *process;
    }
    return out << "}";
}

std::size_t
Process::Set::hash() const
{
    static hash_scope scope;
    hst::hasher hash(scope);
    for (const Process* process : *this) {
        hash.add_unordered(*process);
    }
    return hash.value();
}

void
Process::Set::tau_close()
{
    Event tau = Event::tau();
    while (true) {
        Process::Set new_processes;
        for (const Process* process : *this) {
            process->afters(tau, &new_processes);
        }
        size_type old_size = size();
        insert(new_processes.begin(), new_processes.end());
        size_type new_size = size();
        if (old_size == new_size) {
            return;
        }
    }
}

bool
operator==(const Process::Set& lhs, const Process::Set& rhs)
{
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (const Process* process : lhs) {
        if (rhs.find(process) == rhs.end()) {
            return false;
        }
    }
    return true;
}

std::ostream& operator<<(std::ostream& out, const Process::Set& processes)
{
    // We want reproducible output, so we sort the processes in the set before
    // rendering them into the stream.  We the process's index to print out the
    // processes in the order that they were defined.
    std::vector<const Process*> sorted_processes(processes.begin(),
                                                 processes.end());
    std::sort(sorted_processes.begin(), sorted_processes.end(),
              [](const Process* p1, const Process* p2) {
                  return p1->index() < p2->index();
              });

    bool first = true;
    out << "{";
    for (const Process* process : sorted_processes) {
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

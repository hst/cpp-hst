/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/process.h"

#include <algorithm>
#include <assert.h>
#include <string>
#include <utility>
#include <vector>

#include "hst/hash.h"

namespace hst {

void
Process::initials(Event::Set* out) const
{
    initials([&out](Event event) { out->insert(event); });
}

void
Process::afters(Event initial, Process::Set* out) const
{
    afters(initial, [&out](const Process& process) { out->insert(&process); });
}

void
Process::subprocesses(Process::Set* out) const
{
    subprocesses([&out](const Process& process) { out->insert(&process); });
}

void
NormalizedProcess::afters(Event initial,
                          std::function<void(const Process&)> op) const
{
    const NormalizedProcess* process = after(initial);
    if (process) {
        op(*process);
    }
}

void
Process::Bag::insert(const Process* process)
{
    ++counts_[process];
}

void
Process::Bag::erase(const Process* process)
{
    auto it = counts_.find(process);
    assert(it != counts_.end() && it->second != 0);
    if (--it->second == 0) {
        counts_.erase(it);
    }
}

std::size_t
Process::Bag::hash() const
{
    static hash_scope scope;
    hst::hasher hash(scope);
    for (const Process* process : sorted()) {
        hash.add(*process);
    }
    return hash.value();
}

std::vector<const Process*>
Process::Bag::sorted() const
{
    std::vector<const Process*> sorted;
    for (const auto& process_and_count : *this) {
        const Process* process = process_and_count.first;
        size_type count = process_and_count.second;
        sorted.insert(sorted.end(), count, process);
    }
    std::sort(sorted.begin(), sorted.end(),
              [](const Process* p1, const Process* p2) {
                  return p1->index() < p2->index();
              });
    return sorted;
}

std::ostream& operator<<(std::ostream& out, const Process::Bag& processes)
{
    // We want reproducible output, so we sort the processes in the set before
    // rendering them into the stream.  We use the process's index as the sort
    // key to print out the processes in the order that they were defined.
    bool first = true;
    out << "{";
    for (const Process* process : processes.sorted()) {
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
    for (const Process* process : sorted()) {
        hash.add(*process);
    }
    return hash.value();
}

std::vector<const Process*>
Process::Set::sorted() const
{
    std::vector<const Process*> sorted(begin(), end());
    std::sort(sorted.begin(), sorted.end(),
              [](const Process* p1, const Process* p2) {
                  return p1->index() < p2->index();
              });
    return sorted;
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

std::ostream& operator<<(std::ostream& out, const Process::Set& processes)
{
    // We want reproducible output, so we sort the processes in the set before
    // rendering them into the stream.  We the process's index to print out the
    // processes in the order that they were defined.
    bool first = true;
    out << "{";
    for (const Process* process : processes.sorted()) {
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

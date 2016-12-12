/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst.h"

namespace hst {

std::ostream& operator<<(std::ostream& out, Process process)
{
    return out << process.id_;
}

std::ostream& operator<<(std::ostream& out, const ProcessSet& processes)
{
    out << "{";
    bool first = true;
    for (Process process : processes) {
        if (first) {
            first = false;
            out << process;
        } else {
            out << "," << process;
        }
    }
    return out << "}";
}

LTS::LTS() : stop(0)
{
    stop = add_process();
}

Process
LTS::add_process()
{
    return Process(next_process_id_++);
}

void
LTS::add_transition(Process from, Event event, Process to)
{
    graph_[from][event].insert(to);
}

const LTS::TransitionsMap&
LTS::transitions(Process process) const
{
    auto it = graph_.find(process);
    if (it == graph_.end()) {
        return empty_transitions_;
    }
    return it->second;
}

const ProcessSet&
LTS::afters(Process process, Event initial) const
{
    const TransitionsMap& trans = transitions(process);
    auto it = trans.find(initial);
    if (it == trans.end()) {
        return empty_processes_;
    }
    return it->second;
}

std::ostream&
operator<<(std::ostream& out, const LTS::TransitionsMap& transitions)
{
    out << "{";
    bool first = true;
    for (const auto& transition : transitions) {
        if (first) {
            first = false;
        } else {
            out << ", ";
        }
        out << transition.first << " → " << transition.second;
    }
    return out << "}";
}

}  // namespace hst

/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/semantic-models.h"

#include <algorithm>

#include "hst/event.h"
#include "hst/hash.h"
#include "hst/process.h"

namespace hst {

std::ostream&
operator<<(std::ostream& out, const Trace& trace)
{
    out << "⟨";
    bool first = true;
    for (const Event event : trace) {
        if (first) {
            first = false;
        } else {
            out << ",";
        }
        out << event;
    }
    return out << "⟩";
}

Traces::Behavior
Traces::get_process_behavior(const Process& process)
{
    Event::Set events;
    process.initials(&events);
    events.erase(Event::tau());
    return Traces::Behavior(std::move(events));
}

Traces::Behavior
Traces::get_process_behavior(const Process::Set& processes)
{
    Event::Set events;
    for (const Process* process : processes) {
        process->initials(&events);
    }
    events.erase(Event::tau());
    return Traces::Behavior(std::move(events));
}

bool
Traces::Behavior::refined_by(const Behavior& impl) const
{
    return std::includes(events_.begin(), events_.end(), impl.events_.begin(),
                         impl.events_.end());
}

bool
Traces::Behavior::operator==(const Behavior& other) const
{
    return events_ == other.events_;
}

}  // namespace hst

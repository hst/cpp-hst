/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/semantic-models.h"

#include "hst/event.h"
#include "hst/hash.h"
#include "hst/process.h"

namespace hst {

Traces::Behavior
Traces::get_process_behavior(const Process& process)
{
    Traces::Behavior result;
    process.initials(&result);
    result.erase(Event::tau());
    return result;
}

Traces::Behavior
Traces::get_process_behavior(const Process::Set& processes)
{
    Traces::Behavior result;
    for (const Process* process : processes) {
        process->initials(&result);
    }
    result.erase(Event::tau());
    return result;
}

}  // namespace hst

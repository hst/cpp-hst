/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_SEMANTIC_MODELS_H
#define HST_SEMANTIC_MODELS_H

#include "hst/event.h"
#include "hst/process.h"

namespace hst {

#if 0
// Each semantic model is defined by a struct that must implement the following
// signature:
struct SemanticModel {
    class Behavior;
    static const char* abbreviation();
    static const char* name();
    static Behavior get_process_behavior(const Process& process);
    static Behavior get_process_behavior(const Process::Set& processes);
};
#endif

struct Traces {
    // In the traces model, the behavior of a process is the set of non-τ events
    // that it can perform.
    using Behavior = Event::Set;

    static const char* abbreviation() { return "T"; }
    static const char* name() { return "traces"; }
    static Behavior get_process_behavior(const Process& process);
    static Behavior get_process_behavior(const Process::Set& processes);
};

}  // namespace hst

#endif  // HST_SEMANTIC_MODELS_H

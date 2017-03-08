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
    class Behavior {
      public:
        bool refined_by(const Behavior& impl) const;
    };

    static const char* abbreviation();
    static const char* name();
    static Behavior get_process_behavior(const Process& process);
    static Behavior get_process_behavior(const Process::Set& processes);
};
#endif

struct Traces {
    // In the traces model, the behavior of a process is the set of non-τ events
    // that it can perform.
    class Behavior;

    static const char* abbreviation() { return "T"; }
    static const char* name() { return "traces"; }
    static Behavior get_process_behavior(const Process& process);
    static Behavior get_process_behavior(const Process::Set& processes);
};

class Traces::Behavior {
  public:
    explicit Behavior(Event::Set events) : events_(std::move(events)) {}
    const Event::Set& events() const { return events_; }
    bool refined_by(const Behavior& impl) const;
    bool operator==(const Behavior& other) const;
    bool operator!=(const Behavior& other) const { return !(*this == other); }

  private:
    Event::Set events_;
};

}  // namespace hst

namespace std {

template <>
struct hash<hst::Traces::Behavior>
{
    std::size_t operator()(const hst::Traces::Behavior& behavior) const
    {
        return behavior.events().hash();
    }
};

}  // namespace std

#endif  // HST_SEMANTIC_MODELS_H

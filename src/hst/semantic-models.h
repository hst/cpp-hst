/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_SEMANTIC_MODELS_H
#define HST_SEMANTIC_MODELS_H

#include <ostream>
#include <vector>

#include "hst/environment.h"
#include "hst/event.h"
#include "hst/process.h"

namespace hst {

//------------------------------------------------------------------------------
// Traces

class Trace {
  private:
    using TraceVector = std::vector<Event>;

  public:
    using const_iterator = TraceVector::const_iterator;
    using size_type = TraceVector::size_type;

    // Creates a new empty trace
    Trace() = default;

    explicit Trace(std::vector<Event> events) : events_(std::move(events)) {}

    // Creates a new trace that is an extension of the current one.
    Trace extend(Event suffix) const { return Trace(*this, suffix); }

    const_iterator begin() const { return events_.begin(); }
    const_iterator end() const { return events_.end(); }
    bool empty() const { return events_.empty(); }
    size_type size() const { return events_.size(); }

    bool operator==(const Trace& other) const
    {
        return events_ == other.events_;
    }

    bool operator!=(const Trace& other) const { return !(*this == other); }

  private:
    Trace(const Trace& prefix, Event suffix) : events_(prefix.events_)
    {
        events_.push_back(suffix);
    }

    TraceVector events_;
};

std::ostream&
operator<<(std::ostream& out, const Trace& trace);

template <typename F>
void
find_maximal_finite_traces(Environment* env, const Process* process, F op);

//------------------------------------------------------------------------------
// Semantic models

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

//------------------------------------------------------------------------------
// Internals!

namespace hst {

namespace internal {

struct ProcessList {
    const NormalizedProcess* process;
    ProcessList* prev;

    ProcessList() : process(nullptr), prev(nullptr) {}
    ProcessList(const NormalizedProcess* process, ProcessList* prev)
        : process(process), prev(prev)
    {
    }

    bool contains(const NormalizedProcess* process) const {
        for (const ProcessList* curr = this; curr; curr = curr->prev) {
            if (curr->process == process) {
                return true;
            }
        }
        return false;
    }
};

}  // namespace internal

template <typename F>
void
find_maximal_finite_traces_(const NormalizedProcess* process,
                            internal::ProcessList processes,
                            const Trace& prefix, const F& op)
{
    Event::Set initials;
    process->initials(&initials);

    // If the current process doesn't have any outgoing transitions, we've found
    // the end of a finite trace.
    if (initials.empty()) {
        op(prefix);
        return;
    }

    // If `process` already appears earlier in the current trace, then we've
    // found a cycle.
    if (processes.contains(process)) {
        op(prefix);
        return;
    }

    for (const Event initial : initials) {
        const NormalizedProcess* after = process->after(initial);
        find_maximal_finite_traces_(after,
                                    internal::ProcessList(process, &processes),
                                    prefix.extend(initial), op);
    }
}

template <typename F>
void
find_maximal_finite_traces(Environment* env, const Process* process, F op)
{
    // The prenormalization code can do most of the work for us; it will give us
    // a bunch of subprocesses with at most one outgoing transition for any
    // event.  We then just have to walk through its edges.
    const NormalizedProcess* prenormalized = env->prenormalize(process);
    find_maximal_finite_traces_(prenormalized, internal::ProcessList(), Trace(),
                                op);
}

}  // namespace hst

#endif  // HST_SEMANTIC_MODELS_H

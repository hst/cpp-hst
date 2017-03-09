/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_PROCESS_H
#define HST_PROCESS_H

#include <algorithm>
#include <assert.h>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_set>
#include <vector>

#include "hst/event.h"

namespace hst {

//------------------------------------------------------------------------------
// Process interfaces

class Process {
  public:
    class Bag;
    class Set;
    using Index = unsigned int;

    virtual ~Process() = default;

    Index index() const { return index_; }

    // Fills `out` with the initial events of this process.
    virtual void initials(Event::Set* out) const = 0;

    // Fills `out` with the subprocesses that you reach after following a single
    // `initial` event from this process.
    virtual void afters(Event initial, Set* out) const = 0;

    // Fills `out` with the syntactic subprocesses of this process.  This should
    // only include the subprocesses that are needed to print out the definition
    // of this process.
    virtual void subprocesses(Set* out) const = 0;

    // Calls op for each of the process's outgoing transitions.  op must have a
    // signature compatible with:
    //
    //   bool op(Event initial, const Process* processs)
    //
    // If op ever returns false, then we'll abort the iteration.
    template <typename F>
    void transitions(const F& op) const;

    // Performs a breadth-first search of the reachable subprocesses, calling op
    // for each one.  We guarantee that we'll call op() at most once for each
    // reachable subprocess.  op must have a signature compatible with:
    //
    //   bool op(const Process* process)
    //
    // If op ever returns false, then we'll abort the search.
    template <typename F>
    void bfs(const F& op) const;

    // Performs a breadth-first search of the syntactic subprocesses, calling op
    // for each one.  We guarantee that we'll call op() at most once for each
    // syntactic subprocess.  op must have a signature compatible with:
    //
    //   bool op(const Process* process)
    //
    // If op ever returns false, then we'll abort the search.
    template <typename F>
    void bfs_syntactic(const F& op) const;

    virtual std::size_t hash() const = 0;
    virtual bool operator==(const Process& other) const = 0;
    bool operator!=(const Process& other) const { return !(*this == other); }

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

    template <typename T>
    void print_subprocesses(std::ostream& out, const T& processes,
                            const std::string& binary_op) const;

  private:
    friend class Environment;
    Index index_;
};

inline std::ostream&
operator<<(std::ostream& out, const Process& process)
{
    process.print(out);
    return out;
}

class NormalizedProcess : public Process {
  public:
    virtual const NormalizedProcess* after(Event initial) const = 0;
    void afters(Event initial, Set* out) const final;

    // Returns the set of non-normalized processes that this normalized process
    // represents.
    virtual void expand(Process::Set* out) const = 0;

    // Same as Process::bfs, but op should have a different signature:
    //
    //   bool op(const NormalizedProcess* process)
    template <typename F>
    void bfs(const F& op) const;
};

class Process::Bag : public std::unordered_multiset<const Process*> {
  private:
    using Parent = std::unordered_multiset<const Process*>;

  public:
    using Parent::unordered_multiset;

    std::size_t hash() const;
};

std::ostream& operator<<(std::ostream& out, const Process::Bag& processes);

class Process::Set : public std::unordered_set<const Process*> {
  private:
    using Parent = std::unordered_set<const Process*>;

  public:
    using Parent::unordered_set;

    std::size_t hash() const;

    // Updates this set of processes to be τ-closed.  (That is, we add any
    // additional processes you can reach by following τ one or more times.)
    void tau_close();
};

std::ostream& operator<<(std::ostream& out, const Process::Set& processes);

}  // namespace hst

namespace std {

template <>
struct hash<hst::Process>
{
    std::size_t operator()(const hst::Process& process) const
    {
        return process.hash();
    }
};

template <>
struct hash<hst::Process::Bag>
{
    std::size_t operator()(const hst::Process::Bag& bag) const
    {
        return bag.hash();
    }
};

template <>
struct hash<hst::Process::Set>
{
    std::size_t operator()(const hst::Process::Set& set) const
    {
        return set.hash();
    }
};

}  // namespace std

//------------------------------------------------------------------------------
// Internals!

namespace hst {

template <typename F>
void
Process::transitions(const F& op) const
{
    Event::Set initials;
    this->initials(&initials);
    for (const auto& initial : initials) {
        Process::Set afters;
        this->afters(initial, &afters);
        for (const auto& after : afters) {
            if (!op(initial, after)) {
                return;
            }
        }
    }
}

template <typename F>
void
Process::bfs(const F& op) const
{
    std::unordered_set<const Process*> seen;
    std::unordered_set<const Process*> queue;

    seen.insert(this);
    queue.insert(this);
    while (!queue.empty()) {
        std::unordered_set<const Process*> next_queue;
        for (const Process* process : queue) {
            if (!op(process)) {
                return;
            }
            process->transitions(
                    [&seen, &next_queue](Event initial, const Process* after) {
                        bool was_added = seen.insert(after).second;
                        if (was_added) {
                            next_queue.insert(after);
                        }
                        return true;
                    });
        }
        std::swap(queue, next_queue);
    }
}

template <typename F>
void
NormalizedProcess::bfs(const F& op) const
{
    std::unordered_set<const NormalizedProcess*> seen;
    std::unordered_set<const NormalizedProcess*> queue;
    queue.insert(this);
    while (!queue.empty()) {
        std::unordered_set<const NormalizedProcess*> next_queue;
        for (const NormalizedProcess* process : queue) {
            if (!op(process)) {
                return;
            }
            Event::Set initials;
            process->initials(&initials);
            for (const Event initial : initials) {
                const NormalizedProcess* after = process->after(initial);
                assert(after);
                bool was_added = seen.insert(after).second;
                if (was_added) {
                    next_queue.insert(after);
                }
            }
        }
        std::swap(queue, next_queue);
    }
}

template <typename F>
void
Process::bfs_syntactic(const F& op) const
{
    std::unordered_set<const Process*> seen;
    std::unordered_set<const Process*> queue;

    seen.insert(this);
    queue.insert(this);
    while (!queue.empty()) {
        std::unordered_set<const Process*> next_queue;
        for (const Process* process : queue) {
            if (!op(process)) {
                return;
            }
            Process::Set subprocesses;
            process->subprocesses(&subprocesses);
            for (const Process* subprocess : subprocesses) {
                auto result = seen.insert(subprocess);
                bool added = result.second;
                if (added) {
                    next_queue.insert(subprocess);
                }
            }
        }
        std::swap(queue, next_queue);
    }
}

template <typename T>
void
Process::print_subprocesses(std::ostream& out, const T& processes,
                            const std::string& binary_op) const
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

    if (sorted_processes.size() == 2) {
        const Process* lhs = sorted_processes[0];
        const Process* rhs = sorted_processes[1];
        if (precedence() < lhs->precedence()) {
            out << "(" << *lhs << ")";
        } else {
            out << *lhs;
        }
        out << " " << binary_op << " ";
        if (precedence() < rhs->precedence()) {
            out << "(" << *rhs << ")";
        } else {
            out << *rhs;
        }
        return;
    }

    bool first = true;
    out << binary_op << " {";
    for (const Process* process : sorted_processes) {
        if (first) {
            first = false;
        } else {
            out << ", ";
        }
        out << *process;
    }
    out << "}";
}

}  // namespace hst

#endif  // HST_PROCESS_H

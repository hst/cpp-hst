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
#include <functional>
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

    // Calls `op` for each initial event of this process.  You CAN call `op`
    // multiple times for any given initial event if that makes your
    // implementation easier; it's up to the caller to deduplicate events if
    // they need to.
    virtual void initials(std::function<void(Event)> op) const = 0;

    // Calls `op` for each subprocess that you reach after following a single
    // `initial` event from this process.  You CAN call `op` multiple times for
    // any given process if that makes your implementation easier; it's up to
    // the caller to deduplicate events if they need to.
    virtual void
    afters(Event initial, std::function<void(const Process&)> op) const = 0;

    // Calls `op` for each syntactic subprocesses of this process.  This should
    // only include the subprocesses that are needed to print out the definition
    // of this process.  You CAN call `op` multiple times for any given process
    // if that makes your implementation easier; it's up to the caller to
    // deduplicate events if they need to.
    virtual void subprocesses(std::function<void(const Process&)> op) const = 0;

    // Legacy signatures; only here until we can migrate everything over to the
    // new signatures above.
    void initials(Event::Set* out) const;
    void afters(Event initial, Process::Set* out) const;
    void subprocesses(Process::Set* out) const;

    // Performs a breadth-first search of the reachable subprocesses, calling
    // `op` for each one.  We guarantee that we'll call op() at most once for
    // each reachable subprocess.
    void bfs(std::function<void(const Process&)> op) const;

    // Performs a breadth-first search of the syntactic subprocesses, calling
    // `op` for each one.  We guarantee that we'll call op() at most once for
    // each` syntactic subprocess.
    void bfs_syntactic(std::function<void(const Process&)> op) const;

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
    void
    afters(Event initial, std::function<void(const Process&)> op) const final;

    // Returns the set of non-normalized processes that this normalized process
    // represents.
    virtual void expand(std::function<void(const Process&)> op) const = 0;

    // Same as Process::bfs, but the visitor takes in a NormalizedProcess
    // instead of a Process.
    void bfs(std::function<void(const NormalizedProcess&)> op) const;
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

inline void
Process::bfs(std::function<void(const Process&)> op) const
{
    std::unordered_set<const Process*> seen;
    std::unordered_set<const Process*> queue;
    seen.insert(this);
    queue.insert(this);
    while (!queue.empty()) {
        std::unordered_set<const Process*> next_queue;
        for (const Process* process : queue) {
            op(*process);
            process->initials([process, &op, &seen,
                               &next_queue](Event initial) {
                process->afters(initial, [&op, &seen,
                                          &next_queue](const Process& after) {
                    bool was_added = seen.insert(&after).second;
                    if (was_added) {
                        next_queue.insert(&after);
                    }
                });
            });
        }
        std::swap(queue, next_queue);
    }
}

inline void
NormalizedProcess::bfs(std::function<void(const NormalizedProcess&)> op) const
{
    std::unordered_set<const NormalizedProcess*> seen;
    std::unordered_set<const NormalizedProcess*> queue;
    seen.insert(this);
    queue.insert(this);
    while (!queue.empty()) {
        std::unordered_set<const NormalizedProcess*> next_queue;
        for (const NormalizedProcess* process : queue) {
            op(*process);
            Event::Set initials;
            process->initials([process, &seen, &next_queue](Event initial) {
                const NormalizedProcess* after = process->after(initial);
                assert(after);
                bool was_added = seen.insert(after).second;
                if (was_added) {
                    next_queue.insert(after);
                }
            });
        }
        std::swap(queue, next_queue);
    }
}

inline void
Process::bfs_syntactic(std::function<void(const Process&)> op) const
{
    std::unordered_set<const Process*> seen;
    std::unordered_set<const Process*> queue;
    seen.insert(this);
    queue.insert(this);
    while (!queue.empty()) {
        std::unordered_set<const Process*> next_queue;
        for (const Process* process : queue) {
            op(*process);
            process->subprocesses(
                    [&seen, &next_queue](const Process& subprocess) {
                        bool was_added = seen.insert(&subprocess).second;
                        if (was_added) {
                            next_queue.insert(&subprocess);
                        }
                    });
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

/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_PROCESS_H
#define HST_PROCESS_H

#include <algorithm>
#include <memory>
#include <ostream>
#include <sstream>
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

    virtual ~Process() = default;

    // Fill `out` with the initial events of this process.
    virtual void initials(Event::Set* out) const = 0;

    // Fill `out` with the subprocesses that you reach after following a single
    // `initial` event from this process.
    virtual void afters(Event initial, Set* out) const = 0;

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
};

class Process::Bag : public std::unordered_multiset<const Process*> {
  private:
    using Parent = std::unordered_multiset<const Process*>;

  public:
    using Parent::unordered_multiset;

    std::size_t hash() const;
};

bool
operator==(const Process::Bag& lhs, const Process::Bag& rhs);

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

bool
operator==(const Process::Set& lhs, const Process::Set& rhs);

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

    queue.insert(this);
    while (!queue.empty()) {
        std::unordered_set<const Process*> next_queue;
        for (const Process* process : queue) {
            if (!op(process)) {
                return;
            }
            process->transitions(
                    [&seen, &next_queue](Event initial, const Process* after) {
                        auto result = seen.insert(after);
                        bool added = result.second;
                        if (added) {
                            next_queue.insert(after);
                        }
                        return true;
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
    // We want reproducible output, so we sort the names of the processes in the
    // set before rendering them into the stream.
    std::vector<std::pair<std::string, unsigned int>> process_names;
    for (const auto& process : processes) {
        std::stringstream name;
        name << *process;
        process_names.emplace_back(
                std::make_pair(name.str(), process->precedence()));
    }
    std::sort(process_names.begin(), process_names.end());

    if (process_names.size() == 2) {
        if (precedence() < process_names[0].second) {
            out << "(" << process_names[0].first << ")";
        } else {
            out << process_names[0].first;
        }
        out << " " << binary_op << " ";
        if (precedence() < process_names[1].second) {
            out << "(" << process_names[1].first << ")";
        } else {
            out << process_names[1].first;
        }
        return;
    }

    bool first = true;
    out << binary_op << " {";
    for (const auto& name_and_precedence : process_names) {
        if (first) {
            first = false;
        } else {
            out << ", ";
        }
        out << name_and_precedence.first;
    }
    out << "}";
}

}  // namespace hst

#endif  // HST_PROCESS_H

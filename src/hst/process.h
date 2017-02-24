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
    using Bag = std::unordered_multiset<Process*>;
    using Set = std::unordered_set<Process*>;

    virtual ~Process() = default;

    // Fill `out` with the initial events of this process.
    virtual void initials(Event::Set* out) = 0;

    // Fill `out` with the subprocesses that you reach after following a single
    // `initial` event from this process.
    virtual void afters(Event initial, Set* out) = 0;

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

std::ostream& operator<<(std::ostream& out, const Process::Bag& processes);

bool
operator==(const Process::Bag& lhs, const Process::Bag& rhs);

std::size_t
hash(const Process::Bag& set);

std::ostream& operator<<(std::ostream& out, const Process::Set& processes);

bool
operator==(const Process::Set& lhs, const Process::Set& rhs);

std::size_t
hash(const Process::Set& set);

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
    std::size_t operator()(const hst::Process::Bag& set) const
    {
        return hst::hash(set);
    }
};

template <>
struct hash<hst::Process::Set>
{
    std::size_t operator()(const hst::Process::Set& set) const
    {
        return hst::hash(set);
    }
};

}  // namespace std

//------------------------------------------------------------------------------
// Internals!

namespace hst {

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

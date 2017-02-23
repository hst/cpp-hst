/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/process.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "hst/hash.h"

namespace hst {

bool
operator==(const Process::Bag& lhs, const Process::Bag& rhs)
{
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (const auto& process : lhs) {
        if (rhs.find(process) == rhs.end()) {
            return false;
        }
    }
    return true;
}

std::ostream& operator<<(std::ostream& out, const Process::Bag& processes)
{
    // We want reproducible output, so we sort the names of the processes in the
    // set before rendering them into the stream.
    std::vector<std::string> process_names;
    for (const auto& process : processes) {
        std::stringstream name;
        name << *process;
        process_names.emplace_back(name.str());
    }
    std::sort(process_names.begin(), process_names.end());

    bool first = true;
    out << "{";
    for (auto process : process_names) {
        if (first) {
            first = false;
        } else {
            out << ", ";
        }
        out << process;
    }
    return out << "}";
}

std::size_t
hash(const Process::Bag& set)
{
    static hash_scope scope;
    hasher hash(scope);
    for (const auto& process : set) {
        hash.add_unordered(*process);
    }
    return hash.value();
}

bool
operator==(const Process::Set& lhs, const Process::Set& rhs)
{
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (const auto& process : lhs) {
        if (rhs.find(process) == rhs.end()) {
            return false;
        }
    }
    return true;
}

std::ostream& operator<<(std::ostream& out, const Process::Set& processes)
{
    // We want reproducible output, so we sort the names of the processes in the
    // set before rendering them into the stream.
    std::vector<std::string> process_names;
    for (const auto& process : processes) {
        std::stringstream name;
        name << *process;
        process_names.emplace_back(name.str());
    }
    std::sort(process_names.begin(), process_names.end());

    bool first = true;
    out << "{";
    for (auto process : process_names) {
        if (first) {
            first = false;
        } else {
            out << ", ";
        }
        out << process;
    }
    return out << "}";
}

std::size_t
hash(const Process::Set& set)
{
    static hash_scope scope;
    hasher hash(scope);
    for (const auto& process : set) {
        hash.add_unordered(*process);
    }
    return hash.value();
}

}  // namespace hst

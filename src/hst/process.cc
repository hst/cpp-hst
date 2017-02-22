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
#include <vector>

#include "hst/hash.h"

namespace hst {

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

void
print_set(std::ostream& out, const Process::Set& processes,
          const std::string& binary_op)
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

    if (!binary_op.empty() && process_names.size() == 2) {
        out << process_names[0] << " " << binary_op << " " << process_names[1];
        return;
    }

    if (!binary_op.empty()) {
        out << binary_op << " ";
    }

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
    out << "}";
}

std::ostream& operator<<(std::ostream& out, const Process::Set& processes)
{
    print_set(out, processes);
    return out;
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

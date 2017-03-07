/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/recursion.h"

#include <assert.h>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>

#include "hst/environment.h"
#include "hst/event.h"
#include "hst/hash.h"
#include "hst/process.h"

namespace hst {

RecursionScope
Environment::recursion()
{
    return RecursionScope(this, next_recursion_scope_++);
}

RecursiveProcess*
RecursionScope::add(std::string name)
{
    RecursiveProcess*& process = processes_[name];
    if (!process) {
        process = env_->recursive_process(id_, std::move(name));
    }
    return process;
}

void
RecursionScope::unfilled_processes(std::vector<const std::string*>* names) const
{
    for (const auto& name_and_process : processes_) {
        RecursiveProcess* process = name_and_process.second;
        if (!process->filled()) {
            names->push_back(&process->name());
        }
    }
}

RecursiveProcess*
Environment::recursive_process(RecursionScope::ID scope,
                               const std::string& name)
{
    return register_process(new RecursiveProcess(this, scope, std::move(name)));
}

void
RecursiveProcess::initials(Event::Set* out) const
{
    assert(filled());
    definition_->initials(out);
}

void
RecursiveProcess::afters(Event initial, Process::Set* out) const
{
    assert(filled());
    definition_->afters(initial, out);
}

void
RecursiveProcess::subprocesses(Process::Set* out) const
{
    assert(filled());
    out->insert(definition_);
}

std::size_t
RecursiveProcess::hash() const
{
    static hash_scope recursion;
    return hasher(recursion).add(env_).add(scope_).add(name_).value();
}

bool
RecursiveProcess::operator==(const Process& other_) const
{
    const RecursiveProcess* other =
            dynamic_cast<const RecursiveProcess*>(&other_);
    if (other == nullptr) {
        return false;
    }
    return env_ == other->env_ && scope_ == other->scope_ &&
           name_ == other->name_;
}

namespace {

struct CompareNames {
    bool
    operator()(const RecursiveProcess* p1, const RecursiveProcess* p2) const
    {
        return p1->name() < p2->name();
    }
};

// We need to render a recursive process differently inside of its definition
// and outside of it.  We use this class to detect those two situations.
class StreamWrapper : public std::stringstream {};

}  // namespace

void
RecursiveProcess::print(std::ostream& out) const
{
    // If out is a StreamWrapper, then we're in the middle of printing out the
    // definitions of a recursive process (possibly this one, possibly a
    // different one that this one is mutually recursive with).  In that case,
    // we just need to print out the name of this process.
    if (dynamic_cast<StreamWrapper*>(&out)) {
        out << name();
        return;
    }

    // Otherwise we need to output the `let` statement that contains all of the
    // definitions that are mutually recursive with the current one.  We'll
    // first do a quick BFS to find them all.
    std::set<const RecursiveProcess*, CompareNames> recursive_processes;
    bfs([&recursive_processes](const Process* process) {
        auto recursive_process = dynamic_cast<const RecursiveProcess*>(process);
        if (recursive_process) {
            recursive_processes.insert(recursive_process);
        }
        return true;
    });

    out << "let";
    {
        // Use our helper class here so that we can make sure to render
        // recursive process names in their definitions.
        StreamWrapper helper_out;
        for (const RecursiveProcess* recursive_process : recursive_processes) {
            helper_out << " " << recursive_process->name() << "=";
            helper_out << *recursive_process->definition();
        }
        out << helper_out.rdbuf();
    }
    out << " within " << name();
}

void
RecursiveProcess::fill(const Process* definition)
{
    assert(!filled());
    definition_ = definition;
}

}  // namespace hst

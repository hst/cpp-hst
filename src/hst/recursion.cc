/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/recursion.h"

#include <assert.h>
#include <functional>
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
RecursiveProcess::initials(std::function<void(Event)> op) const
{
    assert(filled());
    definition_->initials(op);
}

void
RecursiveProcess::afters(Event initial,
                         std::function<void(const Process&)> op) const
{
    assert(filled());
    definition_->afters(initial, op);
}

void
RecursiveProcess::subprocesses(std::function<void(const Process&)> op) const
{
    assert(filled());
    op(*definition_);
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

struct CompareIndices {
    bool
    operator()(const RecursiveProcess* p1, const RecursiveProcess* p2) const
    {
        return p1->index() < p2->index();
    }
};

}  // namespace

void
RecursiveProcess::print(std::ostream& out) const
{
    // If out is a StreamWrapper, then we're in the middle of printing out the
    // definitions of a recursive process (possibly this one, possibly a
    // different one that this one is mutually recursive with).  In that case,
    // we just need to print out the name of this process.
    static thread_local bool print_names = false;
    if (print_names) {
        out << name();
        return;
    }

    // Otherwise we need to output the `let` statement that contains all of the
    // definitions that are mutually recursive with the current one.  We'll
    // first do a quick BFS to find them all.
    std::set<const RecursiveProcess*, CompareIndices> recursive_processes;
    bfs_syntactic([&recursive_processes](const Process& process) {
        auto recursive_process =
                dynamic_cast<const RecursiveProcess*>(&process);
        if (recursive_process) {
            recursive_processes.insert(recursive_process);
        }
        return true;
    });

    print_names = true;
    out << "let";
    for (const RecursiveProcess* recursive_process : recursive_processes) {
        out << " " << recursive_process->name() << "=";
        out << *recursive_process->definition();
    }
    out << " within " << name();
    print_names = false;
}

void
RecursiveProcess::fill(const Process* definition)
{
    assert(!filled());
    definition_ = definition;
}

}  // namespace hst

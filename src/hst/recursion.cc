/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/recursion.h"

#include <assert.h>
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
    target_->initials(out);
}

void
RecursiveProcess::afters(Event initial, Process::Set* out) const
{
    assert(filled());
    target_->afters(initial, out);
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

void
RecursiveProcess::print(std::ostream& out) const
{
    out << "whomp";
}

void
RecursiveProcess::fill(const Process* target)
{
    assert(!filled());
    target_ = target;
}

}  // namespace hst

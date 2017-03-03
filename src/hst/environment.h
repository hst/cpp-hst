/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_ENVIRONMENT_H
#define HST_ENVIRONMENT_H

#include <memory>

#include "hst/event.h"
#include "hst/process.h"
#include "hst/recursion.h"

namespace hst {

class Environment {
  public:
    Environment();

    const Process* external_choice(const Process* p, const Process* q);
    const Process* external_choice(Process::Set ps);
    const Process* interleave(const Process* p, const Process* q);
    const Process* interleave(Process::Bag ps);
    const Process* internal_choice(const Process* p, const Process* q);
    const Process* internal_choice(Process::Set ps);
    const Process* prefix(Event a, const Process* p);
    RecursionScope recursion();
    const Process* sequential_composition(const Process* p, const Process* q);
    const Process* skip() const { return skip_; }
    const Process* stop() const { return stop_; }
    const NormalizedProcess* prenormalize(const Process* p);

    // These will typically only be used internally or in test cases.
    RecursiveProcess*
    recursive_process(RecursionScope::ID scope, const std::string& name);
    const NormalizedProcess* prenormalize(Process::Set ps);

    // Ensures that there is exactly one process in the registry equal to
    // `process`, returning a pointer to that process.
    template <typename T>
    T* register_process(T* process);

  private:
    struct deref_hash {
        std::size_t operator()(const std::unique_ptr<Process>& ptr) const
        {
            return ptr->hash();
        }
    };

    struct deref_key_equal {
        bool operator()(const std::unique_ptr<Process>& lhs,
                        const std::unique_ptr<Process>& rhs) const
        {
            return *lhs == *rhs;
        }
    };

    using Registry = std::unordered_set<std::unique_ptr<Process>, deref_hash,
                                        deref_key_equal>;

    Registry registry_;
    const Process* skip_;
    const Process* stop_;
    RecursionScope::ID next_recursion_scope_ = 0;
};

template <typename T>
T*
Environment::register_process(T* process)
{
    std::unique_ptr<T> owned(process);
    auto result = registry_.insert(std::move(owned));
    // This static_cast is safe, even if we're returning an existing process
    // from the registry, since we've already verified that whatever we return
    // is equal to `process`.
    return static_cast<T*>(result.first->get());
}

}  // namespace hst
#endif  // HST_ENVIRONMENT_H

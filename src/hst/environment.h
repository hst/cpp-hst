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
    const Process* sequential_composition(const Process* p, const Process* q);
    const Process* skip() const { return skip_; }
    const Process* stop() const { return stop_; }

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

    // Ensures that there is exactly one process in the registry equal to
    // `process`, returning a pointer to that process.
    const Process* register_process(std::unique_ptr<Process> process);

    Registry registry_;
    const Process* skip_;
    const Process* stop_;
};

}  // namespace hst
#endif  // HST_ENVIRONMENT_H
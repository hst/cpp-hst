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

    Process* external_choice(Process* p, Process* q);
    Process* external_choice(Process::Set ps);
    Process* interleave(Process* p, Process* q);
    Process* interleave(Process::Bag ps);
    Process* internal_choice(Process* p, Process* q);
    Process* internal_choice(Process::Set ps);
    Process* prefix(Event a, Process* p);
    Process* sequential_composition(Process* p, Process* q);
    Process* skip() const { return skip_; }
    Process* stop() const { return stop_; }

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
    Process* register_process(std::unique_ptr<Process> process);

    Registry registry_;
    Process* skip_;
    Process* stop_;
};

}  // namespace hst
#endif  // HST_ENVIRONMENT_H

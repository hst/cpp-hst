/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst.h"

namespace hst {

// Operational semantics for □ Ps
//
//                  P -τ→ P'
//  1)  ────────────────────────────── P ∈ Ps
//       □ Ps -τ→ □ (Ps ∖ {P} ∪ {P'})
//
//         P -a→ P'
//  2)  ───────────── P ∈ Ps, a ≠ τ
//       □ Ps -a→ P'

Process
external_choice(LTS* lts, const ProcessSet& processes)
{
    Process result = lts->add_process();

    for (Process p : processes) {
        for (const auto& transition : lts->transitions(p)) {
            Event initial = transition.first;
            const ProcessSet& afters = transition.second;
            for (Process p_prime : afters) {
                if (initial == Event::tau) {
                    // Handle τ specially:
                    //
                    // afters(□ Ps, τ) =
                    //     ⋃ { □ Ps ∖ {P} ∪ {P'} | P ∈ Ps, P' ∈ afters(P, τ) }
                    //                                                  [rule 1]

                    ProcessSet choices_prime = processes;
                    choices_prime.erase(p);
                    choices_prime.insert(p_prime);
                    Process after_prime = external_choice(lts, choices_prime);
                    lts->add_transition(result, Event::tau, after_prime);
                } else {
                    // afters(□ Ps, a ≠ τ) =
                    //     ⋃ { P' | P ∈ Ps, P' ∈ afters(P, a) }
                    //                                                  [rule 2]
                    lts->add_transition(result, initial, p_prime);
                }
            }
        }
    }

    return result;
}

Process
external_choice(LTS* lts, Process lhs, Process rhs)
{
    ProcessSet processes = {lhs, rhs};
    return external_choice(lts, processes);
}

}  // namespace hst

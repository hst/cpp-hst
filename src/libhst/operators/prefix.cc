/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst.h"

namespace hst {

// Operational semantics for a → P
//
// 1) ─────────────
//     a → P -a→ P

Process
prefix(LTS* lts, Event a, Process b)
{
    Process result = lts->add_process();
    lts->add_transition(result, a, b);
    return result;
}

}  // namespace hst

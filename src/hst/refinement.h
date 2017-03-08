/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_REFINEMENT_H
#define HST_REFINEMENT_H

#include "hst/environment.h"
#include "hst/process.h"
#include "hst/semantic-models.h"

namespace hst {

template <typename Model>
class RefinementChecker {
  public:
    bool refines(const NormalizedProcess* spec, const Process* impl) const;
};

}  // namespace hst

#endif  // HST_REFINEMENT_H

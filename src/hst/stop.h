/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_STOP_H
#define HST_STOP_H

#include "hst/event.h"
#include "hst/process.h"

namespace hst {

class Stop : public Process {
  public:
    static Stop* get();

    void initials(Event::Set* out) override;
    void afters(Event initial, Process::Set* out) override;
    unsigned int precedence() const override { return 1; }
    void print(std::ostream& out) const override;

  private:
    Stop() = default;
};

}  // namespace hst
#endif  // HST_STOP_H

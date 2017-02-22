/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_PREFIX_H
#define HST_PREFIX_H

#include <ostream>
#include <set>

#include "hst/event.h"
#include "hst/process.h"

namespace hst {

class Prefix : public Process {
  public:
    static std::shared_ptr<Prefix> create(Event a, std::shared_ptr<Process> p);

    // Fill `out` with the initial events of this process.
    void initials(Event::Set* out) override;

    // Fill `out` with the subprocesses that you reach after following a single
    // `initial` event from this process.
    void afters(Event initial, Process::Set* out) override;

    unsigned int precedence() const override { return 1; }
    void print(std::ostream& out) const override;

  protected:
    Prefix(Event a, std::shared_ptr<Process> p) : a_(a), p_(p) {}

  private:
    Event a_;
    std::shared_ptr<Process> p_;
};

}  // namespace hst
#endif  // HST_PREFIX_H

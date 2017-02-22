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

    void initials(Event::Set* out) override;
    void afters(Event initial, Process::Set* out) override;

    std::size_t hash() const override;
    bool operator==(const Process& other) const override;
    unsigned int precedence() const override { return 1; }
    void print(std::ostream& out) const override;

  protected:
    Prefix(Event a, std::shared_ptr<Process> p) : a_(a), p_(std::move(p)) {}

  private:
    Event a_;
    std::shared_ptr<Process> p_;
};

}  // namespace hst
#endif  // HST_PREFIX_H

/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_STOP_H
#define HST_STOP_H

#include <memory>

#include "hst/event.h"
#include "hst/process.h"

namespace hst {

class Stop : public Process {
  public:
    static std::shared_ptr<Stop> create();

    void initials(Event::Set* out) override;
    void afters(Event initial, Process::Set* out) override;

    std::size_t hash() const override;
    bool operator==(const Process& other) const override;
    unsigned int precedence() const override { return 1; }
    void print(std::ostream& out) const override;

  protected:
    Stop() = default;
};

}  // namespace hst
#endif  // HST_STOP_H

/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_EXTERNAL_CHOICE_H
#define HST_EXTERNAL_CHOICE_H

#include <memory>

#include "hst/event.h"
#include "hst/process.h"

namespace hst {

class ExternalChoice : public Process {
  public:
    static std::shared_ptr<ExternalChoice>
    create(std::shared_ptr<Process> p, std::shared_ptr<Process> q);

    static std::shared_ptr<ExternalChoice> create(Process::Set ps);

    void initials(Event::Set* out) override;
    void afters(Event initial, Process::Set* out) override;

    std::size_t hash() const override;
    bool operator==(const Process& other) const override;
    unsigned int precedence() const override { return 6; }
    void print(std::ostream& out) const override;

  protected:
    explicit ExternalChoice(Process::Set ps) : ps_(std::move(ps)) {}
    Process::Set ps_;
};

}  // namespace hst
#endif  // HST_EXTERNAL_CHOICE_H

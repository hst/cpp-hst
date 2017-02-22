/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_SEQUENTIAL_COMPOSITION_H
#define HST_SEQUENTIAL_COMPOSITION_H

#include <ostream>
#include <set>

#include "hst/event.h"
#include "hst/process.h"

namespace hst {

class Skip : public Process {
  public:
    static std::shared_ptr<Skip> create();

    void initials(Event::Set* out) override;
    void afters(Event initial, Process::Set* out) override;

    std::size_t hash() const override;
    bool operator==(const Process& other) const override;
    unsigned int precedence() const override { return 1; }
    void print(std::ostream& out) const override;

  protected:
    Skip() = default;
};

class SequentialComposition : public Process {
  public:
    static std::shared_ptr<SequentialComposition>
    create(std::shared_ptr<Process> p, std::shared_ptr<Process> q);

    void initials(Event::Set* out) override;
    void afters(Event initial, Process::Set* out) override;

    std::size_t hash() const override;
    bool operator==(const Process& other) const override;
    unsigned int precedence() const override { return 3; }
    void print(std::ostream& out) const override;

  protected:
    SequentialComposition(std::shared_ptr<Process> p,
                          std::shared_ptr<Process> q)
        : p_(std::move(p)), q_(std::move(q))
    {
    }

  private:
    std::shared_ptr<Process> p_;
    std::shared_ptr<Process> q_;
};

}  // namespace hst
#endif  // HST_SEQUENTIAL_COMPOSITION_H

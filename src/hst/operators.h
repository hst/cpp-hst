/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_OPERATORS_H
#define HST_OPERATORS_H

#include <memory>

#include "hst/event.h"
#include "hst/process.h"

namespace hst {

std::shared_ptr<Process>
external_choice(std::shared_ptr<Process> p, std::shared_ptr<Process> q);

std::shared_ptr<Process>
external_choice(Process::Set ps);

std::shared_ptr<Process>
internal_choice(std::shared_ptr<Process> p, std::shared_ptr<Process> q);

std::shared_ptr<Process>
internal_choice(Process::Set ps);

std::shared_ptr<Process>
prefix(Event a, std::shared_ptr<Process> p);

std::shared_ptr<Process>
sequential_composition(std::shared_ptr<Process> p, std::shared_ptr<Process> q);

std::shared_ptr<Process>
skip();

std::shared_ptr<Process>
stop();

}  // namespace hst
#endif  // HST_OPERATORS_H

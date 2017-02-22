/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/prefix.h"

#include <ostream>
#include <set>

#include "hst/event.h"
#include "hst/hash.h"
#include "hst/process.h"

namespace hst {

class ConcretePrefix : public Prefix {
  public:
    ConcretePrefix(Event a, std::shared_ptr<Process> p)
        : Prefix(a, std::move(p))
    {
    }
};

std::shared_ptr<Prefix>
Prefix::create(Event a, std::shared_ptr<Process> p)
{
    return std::make_shared<ConcretePrefix>(a, std::move(p));
}

void
Prefix::initials(std::set<Event>* out)
{
    out->insert(a_);
}

void
Prefix::afters(Event initial, Process::Set* out)
{
    if (initial == a_) {
        out->insert(p_);
    }
}

std::size_t
Prefix::hash() const
{
    static hash_scope prefix;
    return hasher(prefix).add(a_).add(*p_).value();
}

bool
Prefix::operator==(const Process& other_) const
{
    const Prefix* other = dynamic_cast<const Prefix*>(&other_);
    if (other == nullptr) {
        return false;
    }
    return a_ == other->a_ && *p_ == *other->p_;
}

void
Prefix::print(std::ostream& out) const
{
    out << a_ << " → ";
    print_subprocess(out, *p_);
}

}  // namespace hst

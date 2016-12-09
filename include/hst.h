/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_H
#define HST_H

#include <memory>
#include <ostream>
#include <string>

namespace hst {

//------------------------------------------------------------------------------
// Events

class Event {
  public:
    explicit Event(const std::string& name) : index_(find_or_create_event(name))
    {
    }

    static Event none() { return Event(0); }
    const std::string& name() const;

    bool operator==(const Event& other) const { return index_ == other.index_; }
    bool operator!=(const Event& other) const { return index_ != other.index_; }

  private:
    using Index = unsigned int;
    class Table;

    explicit Event(Index index) : index_(index) {}

    static Index find_or_create_event(const std::string& name);

    static std::unique_ptr<Table> table_;
    Index index_;
};

std::ostream& operator<<(std::ostream& out, const Event& event);

}  // namespace hst
#endif  // HST_H

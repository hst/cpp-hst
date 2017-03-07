/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_EVENT_H
#define HST_EVENT_H

#include <memory>
#include <ostream>
#include <set>
#include <string>

namespace hst {

//------------------------------------------------------------------------------
// Events

class Event {
  public:
    class Set;

    explicit Event(const std::string& name) : index_(find_or_create_event(name))
    {
    }

    static Event none() { return Event(0); }
    const std::string& name() const;

    bool operator==(const Event& other) const { return index_ == other.index_; }
    bool operator!=(const Event& other) const { return index_ != other.index_; }
    bool operator<(const Event& other) const { return index_ < other.index_; }
    bool operator<=(const Event& other) const { return index_ <= other.index_; }
    bool operator>(const Event& other) const { return index_ > other.index_; }
    bool operator>=(const Event& other) const { return index_ >= other.index_; }

    static Event tau()
    {
        static Event tau("τ");
        return tau;
    }

    static Event tick()
    {
        static Event tick("✔");
        return tick;
    }

  private:
    using Index = unsigned int;
    class Table;

    explicit Event(Index index) : index_(index) {}

    static Index find_or_create_event(const std::string& name);

    static std::unique_ptr<Table> table_;
    Index index_;
};

std::ostream& operator<<(std::ostream& out, const Event& event);

class Event::Set : public std::set<Event> {
  private:
    using Parent = std::set<Event>;

  public:
    using Parent::set;
    std::size_t hash() const;
};

std::ostream& operator<<(std::ostream& out, const Event::Set& events);

}  // namespace hst

namespace std {

template <>
struct hash<hst::Event>
{
    std::size_t operator()(const hst::Event& event) const
    {
        return std::hash<string>()(event.name());
    }
};

template <>
struct hash<hst::Event::Set>
{
    std::size_t operator()(const hst::Event::Set& set) const
    {
        return set.hash();
    }
};

}  // namespace std

#endif  // HST_EVENT_H

/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_H
#define HST_H

#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <string>

namespace hst {

//------------------------------------------------------------------------------
// Events

class Event {
  public:
    Event(const char* name) : Event(std::string(name)) {}
    Event(const std::string& name) : index_(find_or_create_event(name)) {}
    static Event none() { return Event(Index(0)); }
    const std::string& name() const;

    bool operator==(const Event& other) const { return index_ == other.index_; }
    bool operator!=(const Event& other) const { return index_ != other.index_; }
    bool operator<(const Event& other) const { return index_ < other.index_; }
    bool operator<=(const Event& other) const { return index_ <= other.index_; }
    bool operator>(const Event& other) const { return index_ > other.index_; }
    bool operator>=(const Event& other) const { return index_ >= other.index_; }

  private:
    using Index = unsigned int;
    class Table;

    explicit Event(Index index) : index_(index) {}

    static Index find_or_create_event(const std::string& name);

    static std::unique_ptr<Table> table_;
    Index index_;
};

std::ostream& operator<<(std::ostream& out, const Event& event);

//------------------------------------------------------------------------------
// Labeled transition systems (LTS)

class Process {
  public:
    bool operator==(const Process& other) const { return id_ == other.id_; }
    bool operator!=(const Process& other) const { return id_ != other.id_; }
    bool operator<(const Process& other) const { return id_ < other.id_; }
    bool operator<=(const Process& other) const { return id_ <= other.id_; }
    bool operator>(const Process& other) const { return id_ > other.id_; }
    bool operator>=(const Process& other) const { return id_ >= other.id_; }

  private:
    friend class LTS;
    friend std::ostream& operator<<(std::ostream& out, Process processes);

    using ID = unsigned int;

    explicit Process(ID id) : id_(id) {}
    ID id_;
};

using ProcessSet = std::set<Process>;

std::ostream& operator<<(std::ostream& out, Process processes);
std::ostream& operator<<(std::ostream& out, const ProcessSet& processes);

class LTS {
  public:
    using TransitionsMap = std::map<Event, ProcessSet>;
    using Graph = std::map<Process, TransitionsMap>;

    Process add_process();
    void add_edge(Process from, Event event, Process to);

    const TransitionsMap& transitions(Process process) const;
    const ProcessSet& afters(Process process, Event initial) const;

  private:
    Process::ID next_process_id_ = 0;
    Graph graph_;
    TransitionsMap empty_transitions_;
    ProcessSet empty_processes_;
};

std::ostream&
operator<<(std::ostream& out, const LTS::TransitionsMap& transitions);

}  // namespace hst
#endif  // HST_H

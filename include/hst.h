/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_H
#define HST_H

#include <cassert>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <string>

namespace hst {

//------------------------------------------------------------------------------
// Results

// Holds either an instance of T or an instance of E.  You'll use this as the
// return value from a function that can return a value on success, or some kind
// of error condition on failure.  T should be the type of the successful return
// value; E should be the type of your error description.
//
// This is inspired by Rust's Result<T,E> and Haskell's Either monad.
template <typename T, typename E>
class Result {
  public:
    Result(const T& success) : valid_(true), value_(std::move(success)) {}
    Result(T&& success) : valid_(true), value_(std::move(success)) {}

    Result(const E& error) : valid_(false), error_(std::move(error)) {}
    Result(E&& error) : valid_(false), error_(std::move(error)) {}

    Result(const Result& other) : valid_(other.valid_)
    {
        if (valid_) {
            new (&value_) T(other.value_);
        } else {
            new (&error_) E(other.error_);
        }
    }

    Result(Result&& other) : valid_(other.valid_)
    {
        if (valid_) {
            new (&value_) T(std::move(other.value_));
        } else {
            new (&error_) E(std::move(other.error_));
        }
    }

    ~Result()
    {
        if (valid_) {
            value_.~T();
        } else {
            error_.~E();
        }
    }

    bool valid() const { return valid_; }
    operator bool() const { return valid_; }

    T& get()
    {
        assert(valid_);
        return value_;
    }

    const T& get() const
    {
        assert(valid_);
        return value_;
    }

    E& get_error()
    {
        assert(!valid_);
        return error_;
    }

    const E& get_error() const
    {
        assert(!valid_);
        return error_;
    }

  private:
    bool valid_;
    union {
        T value_;
        E error_;
    };

    Result() = default;
};

//------------------------------------------------------------------------------
// Events

class Event {
  public:
    Event(const char* name) : Event(std::string(name)) {}
    Event(const std::string& name) : index_(find_or_create_event(name)) {}
    static Event none() { return Event(Index(0)); }
    const std::string& name() const;

    static Event tau;

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

using Alphabet = std::set<Event>;

std::ostream& operator<<(std::ostream& out, Event event);
std::ostream& operator<<(std::ostream& out, const Alphabet& events);

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

    LTS();

    Process add_process();
    void add_transition(Process from, Event event, Process to);

    const TransitionsMap& transitions(Process process) const;
    const ProcessSet& afters(Process process, Event initial) const;

    Process stop;

  private:
    Process::ID next_process_id_ = 0;
    Graph graph_;
    TransitionsMap empty_transitions_;
    ProcessSet empty_processes_;
};

std::ostream&
operator<<(std::ostream& out, const LTS::TransitionsMap& transitions);

//------------------------------------------------------------------------------
// Operators

Process
prefix(LTS* lts, Event a, Process b);

Process
external_choice(LTS* lts, Process lhs, Process rhs);

Process
external_choice(LTS* lts, const ProcessSet& processes);

}  // namespace hst
#endif  // HST_H

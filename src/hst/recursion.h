/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_RECURSION_H
#define HST_RECURSION_H

#include <string>
#include <unordered_map>

#include "hst/event.h"
#include "hst/process.h"

namespace hst {

class Environment;
class RecursiveProcess;

// A "recursion scope" is the main building block that you need to create
// mutually recursive processes.  You can create one or more "recursion targets"
// within the scope, each of which maps a name to a process.  But importantly,
// you don't have to know in advance which process you're going to map each name
// to.  That lets you define a name for a process, and then use that same name
// in the definition of the process.  Presto, recursion!
class RecursionScope {
  public:
    using ID = unsigned int;

    explicit RecursionScope(Environment* env, ID id) : env_(env), id_(id) {}
    RecursiveProcess* add(std::string name);

    // Returns the names of any recursive processes in this scope that haven't
    // been filled.
    void unfilled_processes(std::vector<const std::string*>* names) const;

  private:
    using ProcessMap = std::unordered_map<std::string, RecursiveProcess*>;
    Environment* env_;
    ID id_;
    ProcessMap processes_;
};

class RecursiveProcess : public Process {
  public:
    explicit RecursiveProcess(Environment* env, RecursionScope::ID scope,
                              std::string name)
        : env_(env), scope_(scope), name_(std::move(name)), definition_(nullptr)
    {
    }

    void initials(Event::Set* out) const override;
    void afters(Event initial, Process::Set* out) const override;

    const std::string& name() const { return name_; }
    const Process* definition() const { return definition_; }
    bool filled() const { return definition_; }
    std::size_t hash() const override;
    bool operator==(const Process& other) const override;
    unsigned int precedence() const override { return 0; }
    void print(std::ostream& out) const override;

    // Fills this recursive process with a definition, which must not have
    // already been filed.
    void fill(const Process* definition);

  private:
    Environment* env_;
    RecursionScope::ID scope_;
    std::string name_;
    const Process* definition_;
};

}  // namespace hst
#endif  // HST_RECURSION_H

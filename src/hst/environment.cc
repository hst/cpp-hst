/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/environment.h"

#include "hst/hash.h"

namespace hst {

namespace {

class Skip : public Process {
  public:
    explicit Skip(const Process* stop) : stop_(stop) {}
    void initials(Event::Set* out) const override;
    void afters(Event initial, Process::Set* out) const override;

    std::size_t hash() const override;
    bool operator==(const Process& other) const override;
    unsigned int precedence() const override { return 1; }
    void print(std::ostream& out) const override;

  private:
    const Process* stop_;
};

}  // namespace

void
Skip::initials(Event::Set* out) const
{
    out->insert(Event::tick());
}

void
Skip::afters(Event initial, Process::Set* out) const
{
    if (initial == Event::tick()) {
        out->insert(stop_);
    }
}

std::size_t
Skip::hash() const
{
    static hash_scope skip;
    return hasher(skip).value();
}

bool
Skip::operator==(const Process& other_) const
{
    const Skip* other = dynamic_cast<const Skip*>(&other_);
    if (other == nullptr) {
        return false;
    }
    return true;
}

void
Skip::print(std::ostream& out) const
{
    out << "SKIP";
}

namespace {

class Stop : public Process {
  public:
    Stop() = default;

    void initials(Event::Set* out) const override;
    void afters(Event initial, Process::Set* out) const override;

    std::size_t hash() const override;
    bool operator==(const Process& other) const override;
    unsigned int precedence() const override { return 1; }
    void print(std::ostream& out) const override;
};

}  // namespace

void
Stop::initials(Event::Set* out) const
{
}

void
Stop::afters(Event initial, Process::Set* out) const
{
}

std::size_t
Stop::hash() const
{
    static hash_scope stop;
    return hasher(stop).value();
}

bool
Stop::operator==(const Process& other_) const
{
    const Stop* other = dynamic_cast<const Stop*>(&other_);
    if (other == nullptr) {
        return false;
    }
    return true;
}

void
Stop::print(std::ostream& out) const
{
    out << "STOP";
}

Environment::Environment()
{
    stop_ = register_process(std::unique_ptr<Process>(new Stop));
    skip_ = register_process(std::unique_ptr<Process>(new Skip(stop_)));
}

const Process*
Environment::register_process(std::unique_ptr<Process> process)
{
    auto result = registry_.insert(std::move(process));
    return result.first->get();
}

}  // namespace hst

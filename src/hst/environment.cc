/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/environment.h"

#include <functional>

#include "hst/hash.h"

namespace hst {

namespace {

class Omega : public Process {
  public:
    Omega() = default;

    void initials(std::function<void(Event)> op) const override;
    void afters(Event initial,
                std::function<void(const Process&)> op) const override;
    void subprocesses(std::function<void(const Process&)> op) const override;

    std::size_t hash() const override;
    bool operator==(const Process& other) const override;
    unsigned int precedence() const override { return 1; }
    void print(std::ostream& out) const override;
};

}  // namespace

void
Omega::initials(std::function<void(Event)> op) const
{
}

void
Omega::afters(Event initial, std::function<void(const Process&)> op) const
{
}

void
Omega::subprocesses(std::function<void(const Process&)> op) const
{
}

std::size_t
Omega::hash() const
{
    static hash_scope omega;
    return hasher(omega).value();
}

bool
Omega::operator==(const Process& other_) const
{
    const Omega* other = dynamic_cast<const Omega*>(&other_);
    if (other == nullptr) {
        return false;
    }
    return true;
}

void
Omega::print(std::ostream& out) const
{
    out << "Ω";
}

namespace {

class Skip : public Process {
  public:
    explicit Skip(const Process* omega) : omega_(omega) {}
    void initials(std::function<void(Event)> op) const override;
    void afters(Event initial,
                std::function<void(const Process&)> op) const override;
    void subprocesses(std::function<void(const Process&)> op) const override;

    std::size_t hash() const override;
    bool operator==(const Process& other) const override;
    unsigned int precedence() const override { return 1; }
    void print(std::ostream& out) const override;

  private:
    const Process* omega_;
};

}  // namespace

void
Skip::initials(std::function<void(Event)> op) const
{
    op(Event::tick());
}

void
Skip::afters(Event initial, std::function<void(const Process&)> op) const
{
    if (initial == Event::tick()) {
        op(*omega_);
    }
}

void
Skip::subprocesses(std::function<void(const Process&)> op) const
{
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

    void initials(std::function<void(Event)> op) const override;
    void afters(Event initial,
                std::function<void(const Process&)> op) const override;
    void subprocesses(std::function<void(const Process&)> op) const override;

    std::size_t hash() const override;
    bool operator==(const Process& other) const override;
    unsigned int precedence() const override { return 1; }
    void print(std::ostream& out) const override;
};

}  // namespace

void
Stop::initials(std::function<void(Event)> op) const
{
}

void
Stop::afters(Event initial, std::function<void(const Process&)> op) const
{
}

void
Stop::subprocesses(std::function<void(const Process&)> op) const
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
    omega_ = register_process(new Omega);
    skip_ = register_process(new Skip(omega_));
    stop_ = register_process(new Stop);
}

}  // namespace hst

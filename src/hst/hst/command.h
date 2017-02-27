/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_COMMAND_H
#define HST_COMMAND_H

#include <string>

namespace hst {

class Command {
  public:
    explicit Command(std::string name) : name_(name) {}
    virtual ~Command() = default;
    const std::string& name() const { return name_; }
    virtual void run(int argc, char** argv) = 0;

  private:
    std::string name_;
};

class Reachable : public Command {
  public:
    Reachable() : Command("reachable") {}
    void run(int argc, char** argv) override;
};

}  // namespace hst
#endif  // HST_COMMAND_H

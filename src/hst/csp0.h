/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_CSP0_H
#define HST_CSP0_H

#include <memory>
#include <ostream>
#include <string>

#include "hst/environment.h"
#include "hst/process.h"

namespace hst {

//------------------------------------------------------------------------------
// CSP₀

struct ParseError {
    std::string message;
    void set_message(const std::string& message) { this->message = message; }
};

inline std::ostream&
operator<<(std::ostream& out, const ParseError& error)
{
    return out << error.message;
}

const Process*
load_csp0_string(Environment* env, const std::string& csp0, ParseError* error);

}  // namespace hst
#endif  // HST_CSP0_H

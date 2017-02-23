/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/csp0.h"

#include <cassert>
#include <functional>
#include <string>

#include "hst/event.h"
#include "hst/operators.h"
#include "hst/process.h"

//------------------------------------------------------------------------------
// Debugging nonsense

#if defined(DEBUG_CSP0)
#include <iostream>
#endif

class DebugStream {
  public:
    ~DebugStream() {
#if defined(DEBUG_CSP0)
        std::cerr << std::endl;
#endif
    }

    template <typename T>
    DebugStream& operator<<(const T& value)
    {
#if defined(DEBUG_CSP0)
        std::cerr << value;
#endif
        return *this;
    }
};

static DebugStream
debug()
{
    return DebugStream();
}

struct hex {
    unsigned char ch;
    hex(char ch) : ch(ch) {}
};

#if defined(DEBUG_CSP0)
static std::ostream&
operator<<(std::ostream& out, hex ch)
{
    unsigned char high_nybble = (ch.ch & 0xf0) >> 4;
    unsigned char low_nybble = ch.ch & 0x0f;
    return out << (char) (high_nybble >= 10 ? high_nybble - 10 + 'a' :
                                              high_nybble + '0')
               << (char) (low_nybble >= 10 ? low_nybble - 10 + 'a' :
                                             low_nybble + '0');
}
#endif

//------------------------------------------------------------------------------
// Branch prediction

// TODO: Implement these
#define likely(x) (x)
#define unlikely(x) (x)

//------------------------------------------------------------------------------
// The actual parser

namespace hst {

#define return_if_success(call) \
    do {                        \
        auto __result = (call); \
        if (__result) {         \
            return __result;    \
        }                       \
    } while (0)

#define return_if_error(call)   \
    do {                        \
        auto __result = (call); \
        if (!__result) {        \
            return __result;    \
        }                       \
    } while (0)

class ParseState {
  public:
    explicit ParseState(const std::string& csp0) : ParseState(csp0, nullptr) {}
    ParseState(const std::string& csp0, ParseError* error)
        : parent_(nullptr),
          start_(csp0.data()),
          p_(csp0.data()),
          eof_(p_ + csp0.size()),
          error_(error)
    {
    }

    ~ParseState()
    {
        if (parent_) {
            if (failed_) {
                debug() << "FAIL   " << label_;
            } else {
                debug() << "PASS   " << label_;
            }
        }
        if (parent_ && !failed_) {
            parent_->p_ = p_;
        }
    }

    char get();
    bool get(char* out);

    bool parse_error(const std::string& message)
    {
        fail();
        error_->set_message(message);
        debug() << "ERROR  " << label_ << ": " << message;
        return false;
    }

    bool eof() const { return p_ == eof_; }
    void fail() { failed_ = true; }
    size_t remaining() const { return eof_ - p_; }

    // Return a std::string containing whichever characters were successfully
    // parsed using this ParseState.
    std::string as_string() const { return std::string(start_, p_ - start_); }

    // Returns a new ParseState that lets you implement a limited form of
    // backtracking.  The new ParseState will point at the same position in the
    // original input as this.  Use the new ParseState to attempt to parse some
    // part of the grammar.  If that succeeds, when the new ParseState goes out
    // of scope, it will update this to skip over whatever you successfully
    // parsed out of the new ParseState.  If it fails, call its fail() method
    // before it goes out of scope; that will prevent it from updating this.
    ParseState attempt(const std::string& label)
    {
        return ParseState(this, label, p_, p_, eof_, error_);
    }

  private:
    ParseState(ParseState* parent, const std::string& label, const char* start,
               const char* p, const char* eof, ParseError* error)
        : parent_(parent),
          label_(label),
          start_(start),
          p_(p),
          eof_(eof),
          error_(error)
    {
        debug() << "START  " << label_;
    }

    ParseState* parent_;
    const std::string label_;
    const char* start_;
    const char* p_;
    const char* eof_;
    bool failed_ = false;
    ParseError* error_;
};

inline char
ParseState::get()
{
    assert(p_ < eof_);
    char ch = *p_++;
    if (!label_.empty()) {
        debug() << "[" << hex(ch) << " "
                << (((unsigned char) ch) < 0x80 ? ch : '*') << "] " << label_;
    }
    return ch;
}

inline bool
ParseState::get(char* out)
{
    if (unlikely(p_ == eof_)) {
        return parse_error("Unexpected end of input");
    }
    *out = get();
    return true;
}

template <typename F>
static void
skip_while(ParseState* parent, const std::string& label, F pred)
{
    ParseState state = parent->attempt(label);
    // As long as there are more characters to consume...
    while (likely(!state.eof())) {
        // Try to consume one character.
        ParseState ch = state.attempt(label);
        if (!pred(ch.get())) {
            // If it doesn't match the predicate, then return this character to
            // the stream and return.
            ch.fail();
            return;
        }
    }
}

static void
skip_whitespace(ParseState* state)
{
    skip_while(state, "whitespace", [](char ch) -> bool {
        return ch == ' ' || ch == '\f' || ch == '\n' || ch == '\r' ||
               ch == '\t' || ch == '\v';
    });
}

static bool
is_idstart(char ch)
{
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}

static bool
is_idchar(char ch)
{
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
           (ch >= '0' && ch <= '9') || ch == '_' || ch == '.';
}

static bool
parse_dollar_identifier(ParseState* parent, std::string* out)
{
    ParseState state = parent->attempt("dollar identifier");
    char ch;
    // Parse the leading $
    return_if_error(state.get(&ch));
    if (unlikely(ch != '$')) {
        return state.parse_error("Expected a $");
    }
    // A $ identifier must contain at least one character after the $
    return_if_error(state.get(&ch));
    if (unlikely(!is_idchar(ch))) {
        return state.parse_error("Expected at least one ID characater after $");
    }
    // Parse any additional characters in the identifier
    skip_while(&state, "dollar identifier character", is_idchar);
    *out = state.as_string();
    return true;
}

static bool
parse_regular_identifier(ParseState* parent, std::string* out)
{
    ParseState state = parent->attempt("regular identifier");
    char ch;
    // Parse the leading identifier character
    return_if_error(state.get(&ch));
    if (unlikely(!is_idstart(ch))) {
        return state.parse_error("Expected a letter or underscore");
    }
    // Parse any additional characters in the identifier
    skip_while(&state, "regular identifier character", is_idchar);
    *out = state.as_string();
    return true;
}

static bool
parse_identifier(ParseState* state, std::string* out)
{
    return_if_success(parse_regular_identifier(state, out));
    return_if_success(parse_dollar_identifier(state, out));
    return state->parse_error("Expected an identifier");
}

static bool
require_token(ParseState* parent, const std::string& expected_str)
{
    ParseState expected(expected_str);
    if (unlikely(parent->remaining() < expected.remaining())) {
        return parent->parse_error("Expected " + expected_str);
    }
    ParseState state = parent->attempt(expected_str);
    while (!expected.eof()) {
        if (unlikely(state.get() != expected.get())) {
            return state.parse_error("Expected " + expected_str);
        }
    }
    return true;
}

static bool
parse_process(ParseState* state, std::shared_ptr<Process>* out);

static bool
parse_process_bag(ParseState* parent, Process::Bag* out)
{
    ParseState state = parent->attempt("process bag");
    std::shared_ptr<Process> process;

    return_if_error(require_token(&state, "{"));
    skip_whitespace(&state);
    if (parse_process(&state, &process)) {
        out->insert(std::move(process));
        skip_whitespace(&state);
        while (require_token(&state, ",")) {
            skip_whitespace(&state);
            if (unlikely(!parse_process(&state, &process))) {
                return state.parse_error("Expected process after `,`");
            }
            out->insert(std::move(process));
            skip_whitespace(&state);
        }
    }
    return_if_error(require_token(&state, "}"));
    return true;
}

static bool
parse_process_set(ParseState* parent, Process::Set* out)
{
    ParseState state = parent->attempt("process set");
    std::shared_ptr<Process> process;

    return_if_error(require_token(&state, "{"));
    skip_whitespace(&state);
    if (parse_process(&state, &process)) {
        out->insert(std::move(process));
        skip_whitespace(&state);
        while (require_token(&state, ",")) {
            skip_whitespace(&state);
            if (unlikely(!parse_process(&state, &process))) {
                return state.parse_error("Expected process after `,`");
            }
            out->insert(std::move(process));
            skip_whitespace(&state);
        }
    }
    return_if_error(require_token(&state, "}"));
    return true;
}

// Precedence order (tightest first)
//  1. () STOP SKIP
//  2. → identifier
//  3. ;
//  4. timeout
//  5. interrupt
//  6. □ (infix)
//  7. ⊓ (infix)
//  8. ||
//  9. |||
// 10. \ (hiding)
// 11. replicated operators (prefix)
// 12. let

// Each of these numbered parse_process functions corresponds to one of the
// entries in the precedence order list.

static bool
parse_parenthesized(ParseState* state, std::shared_ptr<Process>* out)
{
    return_if_error(require_token(state, "("));
    skip_whitespace(state);
    return_if_error(parse_process(state, out));
    skip_whitespace(state);
    return_if_error(require_token(state, ")"));
    return true;
}

static bool
parse_stop(ParseState* state, std::shared_ptr<Process>* out)
{
    return_if_error(require_token(state, "STOP"));
    *out = stop();
    return true;
}

static bool
parse_skip(ParseState* state, std::shared_ptr<Process>* out)
{
    return_if_error(require_token(state, "SKIP"));
    *out = skip();
    return true;
}

static bool
parse_process1(ParseState* parent, std::shared_ptr<Process>* out)
{
    // process1 = (process) | STOP | SKIP
    ParseState state = parent->attempt("process1");
    return_if_success(parse_parenthesized(&state, out));
    return_if_success(parse_stop(&state, out));
    return_if_success(parse_skip(&state, out));
    return state.parse_error("Expected (, STOP, or SKIP");
}

static bool
parse_process2(ParseState* parent, std::shared_ptr<Process>* out)
{
    // process2 = process1 | identifier | event → process2
    return_if_success(parse_process1(parent, out));

    ParseState state = parent->attempt("process2");
    std::string id;
    return_if_error(parse_identifier(&state, &id));
    skip_whitespace(&state);

    // prefix
    if (require_token(&state, "->") || require_token(&state, "→")) {
        Event initial(id);
        std::shared_ptr<Process> after;
        skip_whitespace(&state);
        return_if_error(parse_process2(&state, &after));
        *out = prefix(initial, std::move(after));
        return true;
    }

    return state.parse_error("Undefined process " + id);
}

static bool
parse_process3(ParseState* parent, std::shared_ptr<Process>* out)
{
    // process3 = process2 (; process3)?

    std::shared_ptr<Process> lhs;
    return_if_error(parse_process2(parent, &lhs));

    ParseState state = parent->attempt("process3");
    skip_whitespace(&state);
    if (!require_token(&state, ";")) {
        *out = std::move(lhs);
        return true;
    }

    skip_whitespace(&state);
    std::shared_ptr<Process> rhs;
    return_if_error(parse_process3(&state, &rhs));
    *out = sequential_composition(std::move(lhs), std::move(rhs));
    return true;
}

#define parse_process5 parse_process3  // NIY

static bool
parse_process6(ParseState* parent, std::shared_ptr<Process>* out)
{
    std::shared_ptr<Process> lhs;
    // process6 = process5 (⊓ process6)?
    return_if_error(parse_process5(parent, &lhs));

    ParseState state = parent->attempt("process6");
    skip_whitespace(&state);
    if (!require_token(&state, "[]") && !require_token(&state, "□")) {
        *out = std::move(lhs);
        return true;
    }
    skip_whitespace(&state);
    std::shared_ptr<Process> rhs;
    if (!parse_process6(&state, &rhs) != 0) {
        // Expected process after ⊓
        return state.parse_error("Expected process after □");
    }

    *out = external_choice(Process::Set{std::move(lhs), std::move(rhs)});
    return true;
}

static bool
parse_process7(ParseState* parent, std::shared_ptr<Process>* out)
{
    std::shared_ptr<Process> lhs;
    // process7 = process6 (⊓ process7)?
    return_if_error(parse_process6(parent, &lhs));

    ParseState state = parent->attempt("process7");
    skip_whitespace(&state);
    if (!require_token(&state, "|~|") && !require_token(&state, "⊓")) {
        *out = std::move(lhs);
        return true;
    }
    skip_whitespace(&state);
    std::shared_ptr<Process> rhs;
    if (!parse_process7(&state, &rhs) != 0) {
        // Expected process after ⊓
        return state.parse_error("Expected process after ⊓");
    }

    *out = internal_choice(Process::Set{std::move(lhs), std::move(rhs)});
    return true;
}

#define parse_process8 parse_process7  // NIY

static bool
parse_process9(ParseState* parent, std::shared_ptr<Process>* out)
{
    std::shared_ptr<Process> lhs;
    // process9 = process8 (⊓ process9)?
    return_if_error(parse_process8(parent, &lhs));

    ParseState state = parent->attempt("process9");
    skip_whitespace(&state);
    if (!require_token(&state, "|||") && !require_token(&state, "⫴")) {
        *out = std::move(lhs);
        return true;
    }
    skip_whitespace(&state);
    std::shared_ptr<Process> rhs;
    if (!parse_process9(&state, &rhs) != 0) {
        // Expected process after ⫴
        return state.parse_error("Expected process after ⫴");
    }

    *out = interleave(Process::Bag{std::move(lhs), std::move(rhs)});
    return true;
}

#define parse_process10 parse_process9  // NIY

static bool
parse_process11(ParseState* parent, std::shared_ptr<Process>* out)
{
    // process11 = process10 | □ {process} | ⊓ {process}
    ParseState state = parent->attempt("process11");

    // □ {process}
    if (require_token(&state, "[]") || require_token(&state, "□")) {
        skip_whitespace(&state);
        Process::Set processes;
        return_if_error(parse_process_set(&state, &processes));
        *out = external_choice(std::move(processes));
        return true;
    }

    // ⊓ {process}
    if (require_token(&state, "|~|") || require_token(&state, "⊓")) {
        skip_whitespace(&state);
        Process::Set processes;
        return_if_error(parse_process_set(&state, &processes));
        *out = internal_choice(std::move(processes));
        return true;
    }

    // ⫴ {process}
    if (require_token(&state, "|||") || require_token(&state, "⫴")) {
        skip_whitespace(&state);
        Process::Bag processes;
        return_if_error(parse_process_bag(&state, &processes));
        *out = interleave(std::move(processes));
        return true;
    }

    // process10
    return parse_process10(&state, out);
}

static bool
parse_process(ParseState* state, std::shared_ptr<Process>* out)
{
    return parse_process11(state, out);
}

std::shared_ptr<Process>
load_csp0_string(const std::string& csp0, ParseError* error)
{
    ParseState state(csp0, error);
    skip_whitespace(&state);
    debug() << "--- " << csp0;
    std::shared_ptr<Process> result;
    if (unlikely(!parse_process(&state, &result))) {
        return nullptr;
    }
    skip_whitespace(&state);
    if (unlikely(!state.eof())) {
        state.parse_error("Unexpected characters at end of input");
        return nullptr;
    }
    return std::move(result);
}

}  // namespace hst

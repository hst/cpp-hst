/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/csp0.h"

#include <cassert>
#include <cstring>
#include <functional>
#include <string>

#include "hst/environment.h"
#include "hst/event.h"
#include "hst/process.h"
#include "hst/recursion.h"

//------------------------------------------------------------------------------
// Debugging nonsense

// Define DEBUG_CSP0 to 1 to get debugging information about which parser rules
// are being attempted.  Define it to 2 to also get information about which
// individual characters are being parsed.

#if DEBUG_CSP0
#include <iostream>
#endif

namespace {

class DebugStream {
  public:
    ~DebugStream() {
#if DEBUG_CSP0
        std::cerr << std::endl;
#endif
    }

    template <typename T>
    DebugStream& operator<<(const T& value)
    {
#if DEBUG_CSP0
        std::cerr << value;
#endif
        return *this;
    }
};

DebugStream
debug()
{
    return DebugStream();
}

struct hex {
    unsigned char ch;
    hex(char ch) : ch(ch) {}
};

#if DEBUG_CSP0
std::ostream&
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

struct debug_indent {
    int width;
    debug_indent(int width): width(width) {}
};

#if DEBUG_CSP0
std::ostream&
operator<<(std::ostream& out, debug_indent indent)
{
    for (int i = 0; i < indent.width; ++i) {
        out << " ";
    }
    return out;
}
#endif

}  // namespace

//------------------------------------------------------------------------------
// Branch prediction

// TODO: Implement these
#define likely(x) (x)
#define unlikely(x) (x)

//------------------------------------------------------------------------------
// The actual parser

namespace {

// Each rule in the grammar is implemented as a subclass of Parser.  All of the
// actual parsing of a grammar rule should happen in its constructor.  If at any
// point the parsing of a grammar rule fails, call the fail() method to record
// this.  You can use the `attempt` method in nonterminals to try to parse a
// "lower-level" grammar rule; see its documentation for details.
class Parser {
  public:
    // This constructor creates the "top-level" Parser, which contains the full
    // string of data that we want to parse.  The various `load_*` functions
    // down below will call this constructor, and immediately call its `attempt`
    // method with the root grammar rule for whatever language is being parsed.
    explicit Parser(const std::string& csp0)
        : depth_(0),
          label_("parser"),
          p_(csp0.data()),
          eof_(p_ + csp0.size()),
          failed_(false)
    {
        debug() << debug_indent(depth_) << "START " << label_;
    }

    ~Parser() {
        if (failed_) {
            debug() << debug_indent(depth_) << "FAIL " << label_;
        } else {
            debug() << debug_indent(depth_) << "PASS " << label_;
        }
    }

    bool eof() const { return p_ == eof_; }
    bool failed() const { return failed_; }

    // Try to parse another grammar rule at the current position in the input
    // text.  P should be the name of the class that implements the grammar rule
    // that you want to parse.  That class's constructor should take in a
    // Parser* parameter, plus whatever additional parameters it needs (e.g., to
    // store results into).
    //
    // If P parses successfully, we update this parser's state to consume
    // whatever P consumed, and return true.  If P fails to parse, then we
    // return false and nothing else happens.
    //
    // As an example, from within a "parent" Parser, you can call it as follows:
    //
    //     std::string id;
    //     attempt<Identifier>(&id);
    //
    // In this case, the Identifier parser takes in an additional std::string*
    // parameter.  Note that attempt() only takes in the "additional"
    // constructor parameters; the first Parser* parameter will be automatically
    // filled in with the parent parser.
    //
    // A very common pattern will be that if P fails, then the current parser
    // fails as well.  To handle this, just wrap your call to attempt in
    // return_if_failed:
    //
    //     std::string id;
    //     return_if_failed(attempt<Identifier>(&id));
    //
    // which is just syntactic sugar for:
    //
    //     std::string id;
    //     if (!attempt<Identifier>(&id)) {
    //         fail();
    //         return;
    //     }
    template <typename P, typename ...Args>
    bool attempt(Args&&... args);

  protected:
    // This is the superclass constructor that "real" grammar rule subclasses
    // should call; it takes care of propagating the bookkeeping from the parent
    // parser to the child parser.  `label` should be a readable name of the
    // grammar rule you're trying to parse; in debug mode we'll use that to
    // print out some information about which rules are being parsed.
    Parser(Parser* parent, const char* label)
        : depth_(parent->depth_ + 1),
          label_(label),
          p_(parent->p_),
          eof_(parent->eof_),
          failed_(false)
    {
        debug() << debug_indent(depth_) << "START " << label;
    }

    void fail() { failed_ = true; }
    size_t remaining() const { return eof_ - p_; }

    void debug_char(char ch);
    char get();

    int depth_;
    const char* label_;
    const char* p_;
    const char* eof_;
    bool failed_;
};

#define return_if_success(call) \
    do {                        \
        auto __result = (call); \
        if (__result) {         \
            return;             \
        }                       \
    } while (0)

#define return_if_error(call)   \
    do {                        \
        auto __result = (call); \
        if (!__result) {        \
            fail();             \
            return;             \
        }                       \
    } while (0)

inline void
Parser::debug_char(char ch)
{
#if DEBUG_CSP0
    if (DEBUG_CSP0 > 1 && label_) {
        debug() << "[" << hex(ch) << " "
                << (((unsigned char) ch) < 0x80 ? ch : '*') << "] " << label_;
    }
#endif
}

inline char
Parser::get()
{
    assert(p_ < eof_);
    char ch = *p_++;
    debug_char(ch);
    return ch;
}

template <typename P, typename ...Args>
bool
Parser::attempt(Args&&... args)
{
    P parser(this, std::forward<Args>(args)...);
    if (parser.failed()) {
        return false;
    }
    // If the child parser succeeded, update our state to consume all of the
    // data that the child just parsed.
    p_ = parser.p_;
    return true;
}

//------------------------------------------------------------------------------
// Our grammar rules

// Requires there is at least one more character in the input text, and that
// that character satisfies P::predicate().
template <typename P>
class RequireChar : public Parser {
  public:
    RequireChar(Parser* parent, const char* label) : Parser(parent, label)
    {
        if (unlikely(eof() || !P::predicate(get()))) {
            fail();
        }
    }
};

// Requires that a particular string appear next in the input text.
class RequireString : public Parser {
  public:
    RequireString(Parser* parent, const char* expected)
        : Parser(parent, expected)
    {
        size_t length = strlen(expected);
        if (unlikely(remaining() < length)) {
            fail();
            return;
        }

        if (unlikely(memcmp(expected, p_, length) != 0)) {
            fail();
            return;
        }

#if DEBUG_CSP0
        for (const char* ch = expected; ch < expected + length; ++ch) {
            debug_char(*ch);
        }
#endif
        p_ += length;
    }
};

// Skips over any characters in the input text that satisfy P::predicate.  This
// parser will never fail; if there aren't any characters that satisfy the
// predicate, nothing happens.
template <typename P>
class SkipWhile : public Parser {
  public:
    SkipWhile(Parser* parent, const char* label) : Parser(parent, label)
    {
        const char* curr;
        for (curr = p_; curr < eof_ && P::predicate(*curr); ++curr) {
            debug_char(*curr);
        }
        p_ = curr;
    }
};

// Skips over any whitespace in the input text.
class SkipWhitespace : public SkipWhile<SkipWhitespace> {
  public:
    explicit SkipWhitespace(Parser* parent) : SkipWhile(parent, "whitespace") {}
    static bool predicate(char ch)
    {
        return ch == ' ' || ch == '\f' || ch == '\n' || ch == '\r' ||
               ch == '\t' || ch == '\v';
    }
};

// Skips over any digits in the input text.
class SkipDigits : public SkipWhile<SkipDigits> {
  public:
    explicit SkipDigits(Parser* parent) : SkipWhile(parent, "digits") {}
    static bool predicate(char ch) { return ch >= '0' && ch <= '9'; }
};

// Parses a positive integer.
template <typename T>
class Integer : public Parser {
  public:
    Integer(Parser* parent, T* out) : Parser(parent, "integer")
    {
        const char* start = p_;
        return_if_error(attempt<SkipDigits>());
        if (start == p_) {
            // Expected a digit
            fail();
            return;
        }
        T result = 0;
        for (const char* ch = start; ch < p_; ++ch) {
            result *= 10;
            result += (*ch - '0');
        }
        *out = result;
    }
};

// Requires that there is an "identifier character" next in the input text.  (An
// identifier character is one that can be used in an identifier!)
class RequireIDChar : public RequireChar<RequireIDChar> {
  public:
    explicit RequireIDChar(Parser* parent)
        : RequireChar(parent, "identifier character")
    {
    }

    static bool predicate(char ch)
    {
        return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
               (ch >= '0' && ch <= '9') || ch == '_' || ch == '.';
    }
};

// Skips over any identifier characters in the input text.
class SkipIDChar : public SkipWhile<SkipIDChar> {
  public:
    explicit SkipIDChar(Parser* parent)
        : SkipWhile(parent, "identifier character")
    {
    }

    static bool predicate(char ch)
    {
        return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
               (ch >= '0' && ch <= '9') || ch == '_' || ch == '.';
    }
};

// Tries to parse a "dollar identifier" (one that starts with a dollar sign).
// If we succeed, we place the name of the identifier into `out`.  These are
// used when generating a CSP₀ script programmatically; higher level languages
// won't provide these as identifiers that the user can use, and so these are
// available for the generator script to use without having to worry about
// conflicting with user identifiers.
class DollarIdentifier : public Parser {
  public:
    DollarIdentifier(Parser* parent, std::string* out)
        : Parser(parent, "dollar identifier")
    {
        const char* start = p_;
        // Parse the leading $
        return_if_error(attempt<RequireString>("$"));
        // A $ identifier must contain at least one character after the $
        return_if_error(attempt<RequireIDChar>());
        // Parse any additional characters in the identifier
        attempt<SkipIDChar>();
        *out = std::string(start, p_ - start);
    }
};

// Requires that there is an "identifier start character" next in the input
// text.  (This is just like an identifier character, except that identifiers
// can't start with a number.)
class RequireIDStart : public RequireChar<RequireIDStart> {
  public:
    explicit RequireIDStart(Parser* parent)
        : RequireChar(parent, "initial identifier character")
    {
    }

    static bool predicate(char ch)
    {
        return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
               ch == '_';
    }
};

// Tries to parse a regular identifier (one that doesn't start with a dollar
// sign).  If we succeed, we place the name of the identifier into `out`.
class RegularIdentifier : public Parser {
  public:
    RegularIdentifier(Parser* parent, std::string* out)
        : Parser(parent, "regular identifier")
    {
        const char* start = p_;
        // Parse the leading identifier character
        return_if_error(attempt<RequireIDStart>());
        // Parse any additional characters in the identifier
        attempt<SkipIDChar>();
        *out = std::string(start, p_ - start);
    }
};

// Tries to parse an identifier (dollar or regular).  If we succeed, we place
// the name of the identifier into `out`.
class Identifier : public Parser {
  public:
    Identifier(Parser* parent, std::string* out) : Parser(parent, "identifier")
    {
        return_if_success(attempt<RegularIdentifier>(out));
        return_if_success(attempt<DollarIdentifier>(out));
        fail();
    }
};

// Tries to parse a CSP₀ process.  If we succeed, we use `env` to construct the
// corresponding hst::Process object, which we then place into `out`.
//
// Note that CSP₀ and friends have a fairly deep precedence tree; we implement
// this using a separate Parser class for each level of the tree.  You probably
// don't need to care about this if you just need to parse a Process, but if you
// want to look into the details of how this parser works, that's why there's
// some extra complexity.
class Process : public Parser {
  public:
    // We have to forward-declare this constructor since all of the per-level
    // Parsers are mutually recursive.
    Process(Parser* parent, hst::Environment* env, hst::RecursionScope* scope,
            const hst::Process** out);
};

// Tries to parse a set of processes.  This is used by the replicated operators
// to let you (for instance) define an external choice over an arbitrary number
// of processes.
class ProcessSet : public Parser {
  public:
    ProcessSet(Parser* parent, hst::Environment* env,
               hst::RecursionScope* scope, hst::Process::Set* out)
        : Parser(parent, "process set")
    {
        const hst::Process* process;
        return_if_error(attempt<RequireString>("{"));
        return_if_error(attempt<SkipWhitespace>());
        if (attempt<Process>(env, scope, &process)) {
            out->insert(std::move(process));
            return_if_error(attempt<SkipWhitespace>());
            while (attempt<RequireString>(",")) {
                return_if_error(attempt<SkipWhitespace>());
                return_if_error(attempt<Process>(env, scope, &process));
                out->insert(std::move(process));
                return_if_error(attempt<SkipWhitespace>());
            }
        }
        return_if_error(attempt<RequireString>("}"));
    }
};

// Tries to parse a bag of processes.  A bag is just like a set, but can contain
// duplicate entries.  The parser logic looks exactly the same as for
// ProcessSet; the "duplicates are allowed" logic happens down in the
// Process::Bag::insert() method.
class ProcessBag : public Parser {
  public:
    ProcessBag(Parser* parent, hst::Environment* env,
               hst::RecursionScope* scope, hst::Process::Bag* out)
        : Parser(parent, "process bag")
    {
        const hst::Process* process;
        return_if_error(attempt<RequireString>("{"));
        return_if_error(attempt<SkipWhitespace>());
        if (attempt<Process>(env, scope, &process)) {
            out->insert(std::move(process));
            return_if_error(attempt<SkipWhitespace>());
            while (attempt<RequireString>(",")) {
                return_if_error(attempt<SkipWhitespace>());
                return_if_error(attempt<Process>(env, scope, &process));
                out->insert(std::move(process));
                return_if_error(attempt<SkipWhitespace>());
            }
        }
        return_if_error(attempt<RequireString>("}"));
    }
};

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

// Each of these numbered Process classes corresponds to one of the entries in
// the precedence order list.

class ParenthesizedProcess : public Parser {
  public:
    ParenthesizedProcess(Parser* parent, hst::Environment* env,
                         hst::RecursionScope* scope, const hst::Process** out)
        : Parser(parent, "parenthesized process")
    {
        return_if_error(attempt<RequireString>("("));
        return_if_error(attempt<SkipWhitespace>());
        return_if_error(attempt<Process>(env, scope, out));
        return_if_error(attempt<SkipWhitespace>());
        return_if_error(attempt<RequireString>(")"));
    }
};

class Stop : public Parser {
  public:
    Stop(Parser* parent, hst::Environment* env, hst::RecursionScope* scope,
         const hst::Process** out)
        : Parser(parent, "STOP")
    {
        return_if_error(attempt<RequireString>("STOP"));
        *out = env->stop();
    }
};

class Skip : public Parser {
  public:
    Skip(Parser* parent, hst::Environment* env, hst::RecursionScope* scope,
         const hst::Process** out)
        : Parser(parent, "SKIP")
    {
        return_if_error(attempt<RequireString>("SKIP"));
        *out = env->skip();
    }
};

class Process1 : public Parser {
  public:
    Process1(Parser* parent, hst::Environment* env, hst::RecursionScope* scope,
             const hst::Process** out)
        : Parser(parent, "process1")
    {
        // process1 = (process) | STOP | SKIP
        return_if_success(attempt<ParenthesizedProcess>(env, scope, out));
        return_if_success(attempt<Stop>(env, scope, out));
        return_if_success(attempt<Skip>(env, scope, out));
        fail();
    }
};

class Process2 : public Parser {
  public:
    Process2(Parser* parent, hst::Environment* env, hst::RecursionScope* scope,
             const hst::Process** out)
        : Parser(parent, "process2")
    {
        // process2 = process1 | identifier | event → process2
        return_if_success(attempt<Process1>(env, scope, out));

        std::string id;
        return_if_error(attempt<Identifier>(&id));

        // identifier@scope
        if (attempt<RequireString>("@")) {
            hst::RecursionScope::ID scope = 0;
            return_if_error(attempt<Integer<hst::RecursionScope::ID>>(&scope));
            *out = env->recursive_process(scope, std::move(id));
            return;
        }

        return_if_error(attempt<SkipWhitespace>());

        // prefix
        if (attempt<RequireString>("->") || attempt<RequireString>("→")) {
            hst::Event initial(id);
            const hst::Process* after;
            return_if_error(attempt<SkipWhitespace>());
            return_if_error(attempt<Process2>(env, scope, &after));
            *out = env->prefix(initial, std::move(after));
            return;
        }

        // identifier
        if (!scope) {
            // Undefined identifier (not in a let)
            fail();
            return;
        }

        *out = scope->add(std::move(id));
        return;
    }
};

class Process3 : public Parser {
  public:
    Process3(Parser* parent, hst::Environment* env, hst::RecursionScope* scope,
             const hst::Process** out)
        : Parser(parent, "process3")
    {
        // process3 = process2 (; process3)?

        const hst::Process* lhs = nullptr;
        return_if_error(attempt<Process2>(env, scope, &lhs));
        return_if_error(attempt<SkipWhitespace>());
        if (!attempt<RequireString>(";")) {
            *out = std::move(lhs);
            return;
        }

        return_if_error(attempt<SkipWhitespace>());
        const hst::Process* rhs = nullptr;
        return_if_error(attempt<Process3>(env, scope, &rhs));
        *out = env->sequential_composition(std::move(lhs), std::move(rhs));
    }
};

#define Process5 Process3  // NIY

class Process6 : public Parser {
  public:
    Process6(Parser* parent, hst::Environment* env, hst::RecursionScope* scope,
             const hst::Process** out)
        : Parser(parent, "process6")
    {
        // process6 = process5 (⊓ process6)?
        const hst::Process* lhs = nullptr;
        return_if_error(attempt<Process5>(env, scope, &lhs));
        return_if_error(attempt<SkipWhitespace>());
        if (!attempt<RequireString>("[]") && !attempt<RequireString>("□")) {
            *out = std::move(lhs);
            return;
        }

        return_if_error(attempt<SkipWhitespace>());
        const hst::Process* rhs = nullptr;
        return_if_error(attempt<Process6>(env, scope, &rhs));
        *out = env->external_choice(std::move(lhs), std::move(rhs));
    }
};

class Process7 : public Parser {
  public:
    Process7(Parser* parent, hst::Environment* env, hst::RecursionScope* scope,
             const hst::Process** out)
        : Parser(parent, "process7")
    {
        // process7 = process6 (⊓ process7)?
        const hst::Process* lhs = nullptr;
        return_if_error(attempt<Process6>(env, scope, &lhs));
        return_if_error(attempt<SkipWhitespace>());
        if (!attempt<RequireString>("|~|") && !attempt<RequireString>("⊓")) {
            *out = std::move(lhs);
            return;
        }

        return_if_error(attempt<SkipWhitespace>());
        const hst::Process* rhs = nullptr;
        return_if_error(attempt<Process7>(env, scope, &rhs));
        *out = env->internal_choice(std::move(lhs), std::move(rhs));
    }
};

#define Process8 Process7  // NIY

class Process9 : public Parser {
  public:
    Process9(Parser* parent, hst::Environment* env, hst::RecursionScope* scope,
             const hst::Process** out)
        : Parser(parent, "process9")
    {
        // process9 = process8 (⫴ process9)?
        const hst::Process* lhs = nullptr;
        return_if_error(attempt<Process8>(env, scope, &lhs));
        return_if_error(attempt<SkipWhitespace>());
        if (!attempt<RequireString>("|||") && !attempt<RequireString>("⫴")) {
            *out = std::move(lhs);
            return;
        }

        return_if_error(attempt<SkipWhitespace>());
        const hst::Process* rhs = nullptr;
        return_if_error(attempt<Process9>(env, scope, &rhs));
        *out = env->interleave(std::move(lhs), std::move(rhs));
    }
};

#define Process10 Process9  // NIY

class Process11 : public Parser {
  public:
    Process11(Parser* parent, hst::Environment* env, hst::RecursionScope* scope,
              const hst::Process** out)
        : Parser(parent, "process11")
    {
        // process11 = process10 | □ {process} | ⊓ {process}

        // □ {process}
        if (attempt<RequireString>("[]") || attempt<RequireString>("□")) {
            return_if_error(attempt<SkipWhitespace>());
            hst::Process::Set processes;
            return_if_error(attempt<ProcessSet>(env, scope, &processes));
            *out = env->external_choice(std::move(processes));
            return;
        }

        // ⊓ {process}
        if (attempt<RequireString>("|~|") || attempt<RequireString>("⊓")) {
            return_if_error(attempt<SkipWhitespace>());
            hst::Process::Set processes;
            return_if_error(attempt<ProcessSet>(env, scope, &processes));
            *out = env->internal_choice(std::move(processes));
            return;
        }

        // ⫴ {process}
        if (attempt<RequireString>("|||") || attempt<RequireString>("⫴")) {
            return_if_error(attempt<SkipWhitespace>());
            hst::Process::Bag processes;
            return_if_error(attempt<ProcessBag>(env, scope, &processes));
            *out = env->interleave(std::move(processes));
            return;
        }

        return_if_error(attempt<Process10>(env, scope, out));
    }
};

class RecursiveDefinition : public Parser {
  public:
    RecursiveDefinition(Parser* parent, hst::Environment* env,
                        hst::RecursionScope* scope)
        : Parser(parent, "recursive definition")
    {
        std::string id;
        return_if_error(attempt<Identifier>(&id));
        hst::RecursiveProcess* process = scope->add(std::move(id));
        if (process->filled()) {
            // process redefined
            fail();
            return;
        }

        const hst::Process* target;
        return_if_error(attempt<SkipWhitespace>());
        return_if_error(attempt<RequireString>("="));
        return_if_error(attempt<SkipWhitespace>());
        return_if_error(attempt<Process>(env, scope, &target));
        return_if_error(attempt<SkipWhitespace>());
        process->fill(target);
    }
};

class Process12 : public Parser {
  public:
    Process12(Parser* parent, hst::Environment* env, hst::RecursionScope* scope,
              const hst::Process** out)
        : Parser(parent, "process12")
    {
        // process12 = process11 | let [id = process...] within [process]

        // let
        if (attempt<RequireString>("let")) {
            // Create a new recursion scope for this let
            hst::RecursionScope new_scope = env->recursion();

            // Parse the recursion definitions, requiring there to be at least
            // one of them.
            return_if_error(attempt<SkipWhitespace>());
            return_if_error(attempt<RecursiveDefinition>(env, &new_scope));
            return_if_error(attempt<SkipWhitespace>());
            while (!attempt<RequireString>("within")) {
                return_if_error(attempt<RecursiveDefinition>(env, &new_scope));
                return_if_error(attempt<SkipWhitespace>());
            }

            // After parsing the `within`, verify that any identifiers used in
            // the definitions of each of the recursive processes were
            // eventually defined.  (We have to wait to check for that here
            // because we want to allow you to refer to a process that appears
            // later on in the definition with having to forward-declare it.) */
            std::vector<const std::string*> unfilled_names;
            new_scope.unfilled_processes(&unfilled_names);
            if (!unfilled_names.empty()) {
                // TODO: Report which particular processes were undefined.
                fail();
                return;
            }

            // Then parse the let body.
            return_if_error(attempt<SkipWhitespace>());
            return_if_error(attempt<Process>(env, &new_scope, out));
            return;
        }

        return_if_error(attempt<Process11>(env, scope, out));
    }
};

class Process13 : public Parser {
  public:
    Process13(Parser* parent, hst::Environment* env, hst::RecursionScope* scope,
              const hst::Process** out)
        : Parser(parent, "process13")
    {
        // process13 = process12 | prenormalize {process}

        // prenormalize {process}
        if (attempt<RequireString>("prenormalize")) {
            return_if_error(attempt<SkipWhitespace>());
            hst::Process::Set processes;
            return_if_error(attempt<ProcessSet>(env, scope, &processes));
            *out = env->prenormalize(std::move(processes));
            return;
        }

        return_if_error(attempt<Process12>(env, scope, out));
    }
};

// Now that we have all of our per-level Parsers defined, we just have to
// delegate to the top level of the precedence tree to parse a Process.
Process::Process(Parser* parent, hst::Environment* env,
                 hst::RecursionScope* scope, const hst::Process** out)
    : Parser(parent, "process")
{
    return_if_error(attempt<Process13>(env, scope, out));
}

}  // namespace

namespace hst {

const Process*
load_csp0_string(Environment* env, const std::string& csp0, ParseError* error)
{
    debug() << "--- " << csp0;
    Parser parser(csp0);
    parser.attempt<SkipWhitespace>();
    const Process* result;
    if (unlikely(!parser.attempt<::Process>(env, nullptr, &result))) {
        error->set_message("Error parsing CSP₀");
        return nullptr;
    }
    parser.attempt<SkipWhitespace>();
    if (unlikely(!parser.eof())) {
        error->set_message("Unexpected characters at end of input");
        return nullptr;
    }
    return std::move(result);
}

}  // namespace hst

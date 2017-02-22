/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef TEST_CASES_H
#define TEST_CASES_H

#include <iostream>
#include <sstream>
#include <vector>

namespace tests {

//------------------------------------------------------------------------------
// Test cases

// An interface that must be implemented by each step that can be performed
// during a test.
class TestStep {
  public:
    // Execute the step.  Your implementation is responsible for reporting the
    // step's results to stdout in a TAP-compatible format.  `result_index` will
    // be the index of the first result that your step is responsible for.
    virtual void run(unsigned int result_index) = 0;

    // Return the number of TAP results that will be reported by this test.
    virtual unsigned int result_count() = 0;
};

// A list of all of the steps that will be performed during a test.
class TestRegistry {
  public:
    static TestRegistry* get();

    // Add `step` to the list of steps that will be performed.
    void register_step(TestStep* step);

    // Run all of the steps in this registry.
    void run_all();

  private:
    std::vector<TestStep*> steps_;
    unsigned int total_result_count_ = 0;
};


// A test case that produces a single TAP result.
class SingleResultTestCase : public TestStep {
  public:
    // Override this to implement the behavior that you want to test.  If you
    // call fail() at any point during its execution, then we report that the
    // test failed; otherwise we report that it succeeded.
    virtual void body() = 0;

    // Mark that the test has failed.  Returns an ostream that you should fill
    // with the reason for the failure.
    std::ostream& mark_failed();

    // Aborts the current test case.
    void abort();

    // The currently running test case.
    static SingleResultTestCase& current_test_case()
    {
        return *current_test_case_;
    }

    void run(unsigned int index) override;
    unsigned int result_count() override { return 1; }

  protected:
    // `description` should describe what you're testing with this test case; it
    // will be used to construct the success or failure message for TAP.
    explicit SingleResultTestCase(const std::string& description)
        : description_(description)
    {
    }

    static SingleResultTestCase* current_test_case_;

    const std::string description_;
    bool succeeded_;
    std::stringstream failure_reason_;

  private:
    class test_case_aborted : public std::exception {
    };
};

// A test step that prints out a comment without running any actual tests.  This
// is used to create (purely cosmetic) "groups" of test cases.
class TestCaseGroup : public TestStep {
  public:
    TestCaseGroup(const std::string& description, TestRegistry* registry)
        : description_(description)
    {
        registry->register_step(this);
    }

    void run(unsigned int index) override
    {
        std::cout << "# " << description_ << std::endl;
    }

    unsigned int result_count() override { return 0; }

  private:
    const std::string description_;
};

}  // namespace tests

// A helper macro for defining a test case.  Use it as follows:
//
// TEST_CASE("can run a test")
// {
//     // instantiate things and test them
//     fail() << "expected a widget";
// }
#define TEST_CASE(description) TEST_CASE_AT(description, __LINE__)
#define TEST_CASE_AT(description, line) TEST_CASE_AT_(description, line)
#define TEST_CASE_AT_(description, line)                                \
    class TestCase##line : public tests::SingleResultTestCase {         \
      public:                                                           \
        TestCase##line(tests::TestRegistry* registry)                   \
            : tests::SingleResultTestCase(description)                  \
        {                                                               \
            registry->register_step(this);                              \
        }                                                               \
        void body() override;                                           \
    };                                                                  \
    static TestCase##line test_case_##line(tests::TestRegistry::get()); \
    void TestCase##line::body()

// A helper macro for defining a group of test cases.  Use it as follows:
//
// TEST_CASE_GROUP("widgets");
#define TEST_CASE_GROUP(desc) TEST_CASE_GROUP_AT(desc, __LINE__)
#define TEST_CASE_GROUP_AT(desc, line) TEST_CASE_GROUP_AT_(desc, line)
#define TEST_CASE_GROUP_AT_(description, line)                \
    static tests::TestCaseGroup test_case_##line(description, \
                                                 tests::TestRegistry::get());

#define HERE  __FILE__, __LINE__

//------------------------------------------------------------------------------
// Aborting the test

// If you want to abort your test after a failed test, pipe this into fail()
// right after your error message.
class abort_test {
};

std::ostream&
operator<<(std::ostream& out, const abort_test& abort);

//------------------------------------------------------------------------------
// Common checks

// Mark the current test case as failing.
std::ostream&
fail();

// Verify that two values or objects are equal, failing the current test case if
// not.
template <typename T>
void
check_eq(const T& actual, const T& expected)
{
    if (actual != expected) {
        fail() << "Expected " << expected << ", got " << actual << abort_test();
    }
}

// Verify that two values or objects are NOT equal, failing the current test
// case if not.
template <typename T>
void
check_ne(const T& actual, const T& expected)
{
    if (actual == expected) {
        fail() << "Didn't expect " << expected << abort_test();
    }
}

#endif /* TEST_CASES_H */

/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "test-cases.h"

namespace tests {

TestRegistry* TestRegistry::get()
{
    static TestRegistry test_case_registry;
    return &test_case_registry;
}

void TestRegistry::register_step(TestStep* step)
{
    steps_.push_back(step);
    total_result_count_ += step->result_count();
}

void TestRegistry::run_all()
{
    unsigned int result_index = 1;
    std::cout << "1.." << total_result_count_ << std::endl;
    for (auto& step : steps_) {
        step->run(result_index);
        result_index += step->result_count();
    }
}

SingleResultTestCase* SingleResultTestCase::current_test_case_ = nullptr;

void
SingleResultTestCase::run(unsigned int index)
{
    // Assume the test case passes unless we're told otherwise.
    succeeded_ = true;

    // Run the test.
    current_test_case_ = this;
    body();
    current_test_case_ = nullptr;

    // Report the result.
    if (succeeded_) {
        std::cout << "ok " << index << " - " << description_ << std::endl;
    } else {
        std::cout << "not ok " << index << " - " << description_ << std::endl
                  << "# " << failure_reason_.str() << std::endl;
    }
}

std::ostream&
SingleResultTestCase::mark_failed()
{
    succeeded_ = false;
    return failure_reason_;
}

}  // namespace tests

std::ostream&
fail()
{
    return tests::SingleResultTestCase::current_test_case().mark_failed();
}

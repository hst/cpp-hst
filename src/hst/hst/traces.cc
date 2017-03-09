/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/hst/command.h"

#include <getopt.h>
#include <iostream>
#include <string>

#include "hst/csp0.h"
#include "hst/environment.h"
#include "hst/process.h"
#include "hst/semantic-models.h"

namespace hst {

void
TracesCommand::run(int argc, char** argv)
{
    bool verbose = false;
    static struct option options[] = {{"verbose", no_argument, 0, 'v'},
                                      {0, 0, 0, 0}};

    while (true) {
        int option_index = 0;
        int c = getopt_long(argc, argv, "v", options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'v':
                verbose = true;
                break;

            default:
                exit(EXIT_FAILURE);
        }
    }
    argc -= optind, argv += optind;

    if (argc != 1) {
        std::cerr << "Usage: hst traces [-v] <process>" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string csp0((argc--, *argv++));
    Environment env;
    ParseError error;
    const Process* process = load_csp0_string(&env, csp0, &error);
    if (process == nullptr) {
        std::cerr << "Invalid CSP₀ process \"" << csp0 << "\":" << std::endl
                  << error << std::endl;
        exit(EXIT_FAILURE);
    }

    unsigned long count = 0;
    find_maximal_finite_traces(&env, process,
                               [&count, verbose](const Trace& trace) {
                                   if (verbose) {
                                       std::cout << trace << std::endl;
                                   }
                                   count++;
                               });
    if (verbose) {
        std::cout << "Maximal finite traces: ";
    }
    std::cout << count << std::endl;
}

}  // namespace hst

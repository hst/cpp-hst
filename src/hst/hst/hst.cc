/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <cstdlib>
#include <iostream>
#include <vector>

#include "hst/hst/command.h"

static hst::Reachable reachable;
static std::vector<hst::Command*> commands{&reachable};

int
main(int argc, char** argv)
{
    if (argc <= 1) {
        std::cerr << "Usage: hst [command]" << std::endl;
        exit(EXIT_FAILURE);
    }

    argc--, argv++; /* Executable name */
    std::string desired_command(*argv);

    for (hst::Command* command : commands) {
        if (desired_command == command->name()) {
            command->run(argc, argv);
            exit(EXIT_SUCCESS);
        }
    }

    std::cerr << "Unknown command " << desired_command << std::endl;
    exit(EXIT_FAILURE);
}

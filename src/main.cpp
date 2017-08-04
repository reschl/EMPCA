#include "EMPCACore.h"

#include <stdio.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>

int main(int argc, char** argv) {
    if(argc != 2) {
        std::cerr << "Wrong number of arguments" << std::endl;
        return EXIT_FAILURE;
    }

    EMPCACore* core = new EMPCACore();

    STATUS ret = core->start(argv[1]);

    if(ret == STATUS::OK) {
        delete core;
        return EXIT_SUCCESS;
    }

    std::cout << "Error code: " << static_cast<int>(ret) << std::endl;

    delete core;

    return EXIT_FAILURE;
}

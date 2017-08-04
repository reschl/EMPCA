#include "TAPDevice.h"

#include <iostream>

int main() {
	TAPDevice* dev = new TAPDevice(1);

    dev->start();
    std::cin.ignore();
    dev->stop();

	delete dev;

	return 0;
}

/**
 * Displays state information about the beep speaker for development purposes.
 */

#include <cstdint>
#include <iostream>
#include <chrono>
#include <thread>
#include <bitset>
#include "./inpout.hpp"  // InpOut
#include "./constants.hpp"

using namespace constants;

void clear_screen() {
	std::cout << "\033[2J\033[1;1H";
}

int main() {
	InpOut inp_out;

	while (true) {
		clear_screen();
		std::cout << "    NMI control register: "
		          << std::bitset<8>(inp_out.inb(0x61))
				  << "\n";
		std::cout << "           PIT Channel 0: "
		          << std::bitset<8>(inp_out.inb(reg::channel2-2))
		          << std::endl;
		std::cout << "           PIT Channel 2: "
		          << std::bitset<8>(inp_out.inb(reg::channel2))
		          << std::endl;
		std::cout << "   Timer Counter 0 [5:0]: "
		          << std::bitset<8>(inp_out.inb(0x40))
		          << std::endl;
		std::cout << "   Timer Counter 2 [5:0]: "
		          << std::bitset<8>(inp_out.inb(0x42))
		          << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000/60));
	}

	return 0;
}

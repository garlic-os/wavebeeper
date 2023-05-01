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

	// while (true) {
		// clear_screen();
		std::cout << "Keyboard control register: 0b"
		          << std::bitset<8>(inp_out.inb(reg::keyboard_control))
		          << std::endl;
		std::cout << "            PIT Channel 2: 0b"
		          << std::bitset<8>(inp_out.inb(reg::channel2))
		          << std::endl;
		std::cout << "                 PIT Mode: 0b"
		          << std::bitset<8>(inp_out.inb(reg::mode))
		          << std::endl;
		// std::this_thread::sleep_for(std::chrono::milliseconds(1000/60));
	// }

	return 0;
}

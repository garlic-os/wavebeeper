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

bool test_bit(uint8_t bit, uint8_t byte) {
	return (byte & (1 << bit)) != 0;
}

int main() {
	InpOut inp_out;

	uint16_t pit2_readout;
	uint8_t nmi_readout;
	char ugh[2] = {' ', ' '};

	while (true) {
		// Read PIT Channel 2 once for the low byte,
		// then again for the high byte.
		pit2_readout = inp_out.inb(reg::channel2);
		pit2_readout |= inp_out.inb(reg::channel2) << 8;

		nmi_readout = inp_out.inb(reg::nmi_control);

		if (test_bit(flag::nmi::tmr2_out_sts, nmi_readout)){
			ugh[1] = 'X';
		} else {
			ugh[1] = ' ';
		}

		std::cout << std::bitset<8>(nmi_readout)
		          << "    "
				  << std::bitset<16>(pit2_readout)
				  << ugh
		          << std::endl;

		std::this_thread::sleep_for(std::chrono::milliseconds(1000/60));
	}

	return 0;
}

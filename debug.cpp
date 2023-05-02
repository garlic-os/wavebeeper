#include <cstdint>
#include <cmath>       // std::round
#include <chrono>
#include <thread>
#include "./inpout.hpp"  // InpOut
#include "./constants.hpp"

using namespace constants;


InpOut m_inp_out;


uint16_t freq2period(double freq) {
	return static_cast<uint16_t>(std::round(pit_tick_rate / freq));
}

void configure_pit() {
	m_inp_out.outb(
		constants::flag::pit_mode::select::channel2 |
		constants::flag::pit_mode::access_mode::lobyte_then_hibyte |
		constants::flag::pit_mode::operating_mode::square_wave_generator |
		constants::flag::pit_mode::binary_mode::binary,
		reg::pit_mode
	);
}

void set_pit_freq(double freq) {
	uint16_t period = freq2period(freq);
	m_inp_out.outb(static_cast<uint8_t>(period & 0xFF), reg::channel2);
	m_inp_out.outb(static_cast<uint8_t>(period >> 8), reg::channel2);
}

void pit_wait() {
	while (!m_inp_out.test_bit(flag::nmi::tmr2_out_sts, 0x61)) {}
}


int main() {
	configure_pit();
	set_pit_freq(440.0);
	for (int i = 0; i < 100; i++) {
		// m_inp_out.set_bit(0, 0x61);
		m_inp_out.set_bit(1, 0x61);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		// m_inp_out.clear_bit(0, 0x61);
		m_inp_out.clear_bit(1, 0x61);
	}

	return 0;
}

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <cstdint>

namespace constants {
	// http://www.osdever.net/bkerndev/Docs/pit.htm
	constexpr uint32_t pit_tick_rate = 1193180;

	// https://wiki.osdev.org/Programmable_Interval_Timer#I.2FO_Ports
	namespace reg {
		constexpr uint8_t channel2 = 0x42;
		constexpr uint8_t mode = 0x43;

		// https://wiki.osdev.org/PC_Speaker#The_Raw_Hardware
		// 13.7.1 https://www.intel.com/content/dam/www/public/us/en/documents/datasheets/7-series-chipset-pch-datasheet.pdf
		constexpr uint8_t keyboard_control = 0x61;
	}

	// https://wiki.osdev.org/Programmable_Interval_Timer#Operating_Modes
	namespace flag {
		namespace mode {
			namespace select {
				constexpr uint8_t channel0 = 0b00000000;
				constexpr uint8_t channel1 = 0b01000000;
				constexpr uint8_t channel2 = 0b10000000;
				constexpr uint8_t readback = 0b11000000;
			}
			namespace access_mode {
				constexpr uint8_t latch_count_value_command = 0b00000000;
				constexpr uint8_t lobyte_only = 0b00010000;
				constexpr uint8_t hibyte_only = 0b00100000;
				constexpr uint8_t lobyte_then_hibyte = 0b00110000;
			}
			namespace operating_mode {
				constexpr uint8_t interrupt_on_terminal_count = 0b00000000;
				constexpr uint8_t hardware_retriggerable_one_shot = 0b00000010;
				constexpr uint8_t rate_generator = 0b00000100;
				constexpr uint8_t square_wave_generator = 0b00000110;
				constexpr uint8_t software_triggered_strobe = 0b00001000;
				constexpr uint8_t hardware_triggered_strobe = 0b00001010;
			}
			namespace binary_mode {
				constexpr uint8_t binary = 0b00000000;
				constexpr uint8_t bcd = 0b00000001;
			}
		}
		namespace keyboard {
			constexpr uint8_t speaker_enable = 0b00000010;
			constexpr uint8_t gate2 = 0b00000001;
		}
	}
}


#endif  // CONSTANTS_HPP

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <cstdint>

namespace constants {
	// http://www.osdever.net/bkerndev/Docs/pit.htm
	constexpr uint32_t pit_tick_rate = 1193180;

	// https://wiki.osdev.org/Programmable_Interval_Timer#I.2FO_Ports
	namespace reg {
		constexpr uint8_t channel2 = 0x42;
		constexpr uint8_t pit_mode = 0x43;

		// 13.7.1 (p. 522) https://www.intel.com/content/dam/www/public/us/en/documents/datasheets/7-series-chipset-pch-datasheet.pdf
		constexpr uint8_t nmi_control = 0x61;
	}

	// https://wiki.osdev.org/Programmable_Interval_Timer#Operating_Modes
	namespace flag {
		namespace pit_mode {
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

		namespace nmi {
			constexpr uint8_t serrno_nmi_source_status = 7;
			constexpr uint8_t iochk_nmi_sts = 6;
			constexpr uint8_t tmr2_out_sts = 5;
			constexpr uint8_t ref_toggle = 4;
			constexpr uint8_t iochk_nmi_en = 3;
			constexpr uint8_t pci_serr_en = 2;
			constexpr uint8_t spkr_dat_en = 1;
			constexpr uint8_t tim_cnt2_en = 0;
		}
	}
}


#endif  // CONSTANTS_HPP

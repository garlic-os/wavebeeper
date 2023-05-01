#include <iostream>    // std::cin, std::istream
#include <cmath>       // std::round
#include <cstdint>     // uint8_t, uint16_t, uint32_t
#include <cstring>     // strncmp
#include "./constants.hpp"
#include "./inpout.hpp"  // InpOut
#include "./windows-setup.hpp"

using namespace constants;


/**
 * Plays a stream of a SQR file through the beep speaker.
 * 
 * @pre: The input stream is open in binary mode.
 *       The input stream's cursor is at the start of a valid SQR file.
 */
class SQRPlayer {
  private:
	InpOut m_inp_out;  // An InpOut instance for accessing the beep speaker and PIT.
	std::istream& m_input;  // The input stream to read from (e.g. cin or a file).
	uint32_t m_sample_rate;  // Samples per second (i.e. Hz).
	uint32_t m_data_size;  // The reported size of the SQR file's data section.


	// Move the beep speaker's diaphragm outward (excited state).
	// https://wiki.osdev.org/PC_Speaker#The_Raw_Hardware
	void beeper_out() const {
		m_inp_out.set_bit(1, reg::keyboard_control);
	}

	// Move the beep speaker's diaphragm inward (relaxed state).
	void beeper_in() const {
		m_inp_out.clear_bit(1, reg::keyboard_control);
	}


	uint16_t freq2period(double freq) const {
		return static_cast<uint16_t>(std::round(pit_tick_rate / freq));
	}

	// Set the frequency of PIT Channel 2.
	void set_pit_freq(double freq) const {
		// Set Channel 2 to Square Wave mode and prepare it to accept a 16-bit
		// reload value.
		m_inp_out.outb(
			constants::flag::mode::select::channel2 |
			constants::flag::mode::access_mode::lobyte_then_hibyte |
			constants::flag::mode::operating_mode::square_wave_generator |
			constants::flag::mode::binary_mode::binary,
			reg::mode
		);

		// Set the channel's reload value to the period corresponding to the
		// given frequency.
		uint16_t period = freq2period(freq);
		m_inp_out.outb(reg::channel2, static_cast<uint8_t>(period && 0xFF));
		m_inp_out.outb(reg::channel2, static_cast<uint8_t>(period >> 8));
	}

	// Wait for PIT Channel 2 to complete its current cycle.
	// TODO: Use interrupts instead of polling.
	void pit_wait() const {
		// The PIT alternates between some large value (not necessarily 0xFF)
		// and some low value (not necessarily 0x00).
		while (m_inp_out.inb(reg::channel2) < 43) {}
	}


	// I forgot to write down what this does but the PIT doesn't cycle without it
	void init_something() const {
		// m_inp_out.outb(m_inp_out.inb(0x64) & 0b11101111, 0x64);
		// m_inp_out.outb(0x60, 0x64);
		// uint8_t guh_mode = m_inp_out.inb(0x60);
		// m_inp_out.outb(0x60, 0x64);
		// m_inp_out.outb(guh_mode | 0b00000001, 0x60);
		m_inp_out.outb(
			m_inp_out.inb(reg::keyboard_control) | 0b00000001,
			reg::keyboard_control
		);
	}

	void deinit_something() const {
		// m_inp_out.outb(m_inp_out.inb(0x64) | 0b00010000, 0x64);
		m_inp_out.outb(
			m_inp_out.inb(reg::keyboard_control) & 0b00001110,
			reg::keyboard_control
		);
	}


  public:
	explicit SQRPlayer(std::istream& input_stream) : m_input(input_stream) {
		// Verify that the input is a valid SQR file.
		char buffer[4];
		m_input.read(buffer, 4);
		if (strncmp(buffer, "SQR\x00", 4) != 0) {
			throw "Invalid SQR file";
		}

		// Read the metadata.
		m_input.read(reinterpret_cast<char*>(&m_sample_rate), 4);
		m_input.read(reinterpret_cast<char*>(&m_data_size), 4);
	}


	virtual ~SQRPlayer() {
		beeper_in();
	}


	/**
	 * @post: The input stream's cursor is at the end of the SQR file's data
	 *        section or the EOF, whichever is encountered first.
	 */
	void play() {
		uint32_t bytes_read;
		char sample_byte;
		uint8_t bit_index;

		// Set Channel 2 of the PIT to cycle at the given sample rate.
		set_pit_freq(m_sample_rate);
		// init_something();

		// Read samples in groups of 8 (1 byte) and play them.
		for (bytes_read = 0;
			 bytes_read < m_data_size && m_input.get(sample_byte);
			 ++bytes_read) {
			for (bit_index = 0; bit_index < 8; ++bit_index) {
				if ((sample_byte >> (7 - bit_index)) & 1) {
					// Bit is a 1, so move the speaker's diaphragm outward.
					beeper_out();
				} else {
					// Bit is a 0, so move the speaker's diaphragm inward.
					beeper_in();
				}
				pit_wait();  // Wait for the PIT to complete its current cycle,
				             // which happens every sample-rate-th of a second.
			}
		}

		beeper_in();
		// deinit_something();
	}
};


int main() {
	windows_setup();
	SQRPlayer player(std::cin);
	player.play();
	return 0;
}

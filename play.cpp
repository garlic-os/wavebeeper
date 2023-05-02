#include <iostream>    // std::cin, std::istream
#include <cmath>       // std::round
#include <cstdint>     // uint8_t, uint16_t, uint32_t
#include <cstring>     // strncmp
#include "./constants.hpp"
#include "./inpout.hpp"  // InpOut
#include "./windows-setup.hpp"

using namespace constants;

bool test_bit(uint8_t bit, uint8_t byte) {
	return (byte & (1 << bit)) != 0;
}


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
		m_inp_out.set_bit(1, reg::nmi_control);
	}

	// Move the beep speaker's diaphragm inward (relaxed state).
	void beeper_in() const {
		m_inp_out.clear_bit(1, reg::nmi_control);
	}


	static uint16_t freq2period(double freq) {
		return static_cast<uint16_t>(std::round(pit_tick_rate / freq));
	}

	static uint8_t low_byte(uint16_t value) {
		return static_cast<uint8_t>(value & 0xFF);
	}

	static uint8_t high_byte(uint16_t value) {
		return static_cast<uint8_t>(value >> 8);
	}


	// Set Channel 2 to Square Wave mode and prepare it to accept a 16-bit
	// reload value.
	void configure_pit() const {
		m_inp_out.outb(
			constants::flag::pit_mode::select::channel2 |
			constants::flag::pit_mode::access_mode::lobyte_then_hibyte |
			constants::flag::pit_mode::operating_mode::square_wave_generator |
			constants::flag::pit_mode::binary_mode::binary,
			reg::pit_mode
		);
	}

	// Set the frequency of PIT Channel 2.
	void set_pit_freq(double freq) const {
		// Set the channel's reload value to the period corresponding to the
		// given frequency.
		uint16_t period = freq2period(freq);
		m_inp_out.outb(low_byte(period & 0xFF), reg::channel2);
		m_inp_out.outb(high_byte(period >> 8), reg::channel2);
	}

	// Wait for PIT Channel 2 to complete its current cycle.
	// TODO: Use interrupts instead of polling.
	void pit_wait() const {
		while (!m_inp_out.test_bit(flag::nmi::tmr2_out_sts, reg::nmi_control)) {}
	}

	void init() {
		m_inp_out.set_bit(0, reg::nmi_control);
	}

	void deinit() {
		m_inp_out.clear_bit(0, reg::nmi_control);
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
		deinit();
	}


	/**
	 * @post: The input stream's cursor is at the end of the SQR file's data
	 *        section or the EOF, whichever is encountered first.
	 */
	void play() {
		uint32_t bytes_read;
		char sample_byte;
		uint8_t bit_index;

		init();

		// Set Channel 2 of the PIT to cycle at the given sample rate.
		configure_pit();
		set_pit_freq(m_sample_rate);

		// Read samples in groups of 8 (1 byte) and play them.
		for (bytes_read = 0;
			 bytes_read < m_data_size && m_input.get(sample_byte);
			 ++bytes_read) {
			for (bit_index = 0; bit_index < 8; ++bit_index) {
				if (test_bit(bit_index, sample_byte)) {
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
		deinit();
	}
};


int main() {
	windows_setup();
	SQRPlayer player(std::cin);
	player.play();
	return 0;
}

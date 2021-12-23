#include <stdio.h>     // stdin
#include <fcntl.h>     // _setmode, _O_BINARY
#include <io.h>        // _fileno
#include <iostream>    // std::cin, std::istream
#include <cmath>       // std::round
#include <cstdint>     // uint8_t, uint16_t, uint32_t
#include "inpout.hpp"  // InpOut


/**
 * Plays a stream of a SQR file through the beep speaker.
 */
class SQRPlayer {
 private:
  // https://wiki.osdev.org/Programmable_Interval_Timer#I.2FO_Ports
  static constexpr uint32_t PIT_TICKRATE = 1193180;
  static constexpr uint8_t PIT_CHANNEL_2 = 0x42;
  static constexpr uint8_t PIT_CMDREG = 0x43;

  // https://wiki.osdev.org/PC_Speaker#The_Raw_Hardware
  static constexpr uint8_t KB_CONTROL_PORT = 0x61;


  InpOut& m_inp_out = InpOut();  // An InpOut instance for accessing the beep
                                 // speaker and PIT.
  std::istream& m_input;  // The input stream to read from (e.g. cin or a file).
  uint32_t m_sample_rate;  // Samples per second (i.e. Hz).
  uint32_t m_data_size;  // The reported size of the SQR file's data section.


  // Move the beep speaker's diaphragm outward (excited state).
  // https://wiki.osdev.org/PC_Speaker#The_Raw_Hardware
  void beeper_out() const {
    // Set bit 1 of the keyboard control register.
    m_inp_out.outb(m_inp_out.inb(KB_CONTROL_PORT) | 0b00000010,
                   KB_CONTROL_PORT);
  }


  // Move the beep speaker's diaphragm inward (relaxed state).
  void beeper_in() const {
    // Clear bit 1 of the keyboard control register.
    m_inp_out.outb(m_inp_out.inb(KB_CONTROL_PORT) & 0b11111101,
                   KB_CONTROL_PORT);
  }


  uint16_t freq2period(double freq) {
    return static_cast<uint16_t>(std::round(PIT_TICKRATE / freq));
  }

  // Set the frequency of Channel 2 of the PIT.
  void set_pit_freq(double freq) {
    uint16_t period = freq2period(freq);

    // Set Channel 2 to Square Wave mode.
    // https://wiki.osdev.org/Programmable_Interval_Timer#Operating_Modes
    m_inp_out.outb(PIT_CMDREG, 0b10110110);

    // Set the channel's reload value to the period corresponding to the given
    // frequency.
    m_inp_out.outb(PIT_CHANNEL_2, (uint8_t) period);
    m_inp_out.outb(PIT_CHANNEL_2, (uint8_t) (period >> 8));
  }


  void pit_wait() const {
    while (m_inp_out.inb(PIT_CHANNEL_2) < 127) {}
  }


 public:
  /**
   * @pre: The input stream is open in binary mode.
   *       The input stream's cursor is at the start of a valid SQR file.
   */
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


    set_pit_freq(m_sample_rate);
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
  }
};


int main() {
  if (!_setmode(_fileno(stdin), _O_BINARY)) {
    throw "Failed to set stdin to binary mode";
  }

  SQRPlayer player(std::cin);
  player.play();
  return 0;
}

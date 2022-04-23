#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
// Set stdin to binary mode on Windows
#include <stdio.h>     // stdin
#include <fcntl.h>     // _setmode, _O_BINARY
#include <io.h>        // _fileno
#endif

#include <iostream>    // std::cin, std::istream
#include <cmath>       // std::round
#include <cstdint>     // uint8_t, uint16_t, uint32_t
#include "inpout.hpp"  // InpOut


/**
 * Plays a stream of a SQR file through the beep speaker.
 * 
 * @pre: The input stream is open in binary mode.
 *       The input stream's cursor is at the start of a valid SQR file.
 */
class SQRPlayer {
 private:
  // https://wiki.osdev.org/Programmable_Interval_Timer#I.2FO_Ports
  static constexpr uint32_t PIT_TICKRATE = 1193180;
  static constexpr uint8_t PIT_CHANNEL_2 = 0x42;
  static constexpr uint8_t PIT_CMDREG = 0x43;

  // https://wiki.osdev.org/PC_Speaker#The_Raw_Hardware
  // 13.7.1 https://www.intel.com/content/dam/www/public/us/en/documents/datasheets/7-series-chipset-pch-datasheet.pdf
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


  uint16_t freq2period(double freq) const {
    return static_cast<uint16_t>(std::round(PIT_TICKRATE / freq));
  }

  // Set the frequency of Channel 2 of the PIT.
  void set_pit_freq(double freq) const {
    // Set Channel 2 to Square Wave mode.
    // https://wiki.osdev.org/Programmable_Interval_Timer#Operating_Modes
    m_inp_out.outb(PIT_CMDREG, 0b10110110);

    // Set the channel's reload value to the period corresponding to the given
    // frequency.
    uint16_t period = freq2period(freq);
    m_inp_out.outb(PIT_CHANNEL_2, static_cast<uint8_t>(period));
    m_inp_out.outb(PIT_CHANNEL_2, static_cast<uint8_t>(period >> 8));
  }

  // Wait for Channel 2 of the PIT to complete its current cycle.
  void pit_wait() const {
    // The PIT cycles between some large value (not necessarily 0xFF) and some
    // low value (not necessarily 0x00).
    while (m_inp_out.inb(PIT_CHANNEL_2) < 127) {}
  }


  // I forgot to write down what this does but the PIT doesn't cycle without it
  void init_something() const {
    // m_inp_out.outb(m_inp_out.inb(0x64) & 0b11101111, 0x64);
    // m_inp_out.outb(0x60, 0x64);
    // uint8_t guh_mode = m_inp_out.inb(0x60);
    // m_inp_out.outb(0x60, 0x64);
    // m_inp_out.outb(guh_mode | 0b00000001, 0x60);
    m_inp_out.outb(m_inp_out.inb(KB_CONTROL_PORT) | 0b00000001,
                   KB_CONTROL_PORT);
  }

  void deinit_something() const {
    // m_inp_out.outb(m_inp_out.inb(0x64) | 0b00010000, 0x64);
    m_inp_out.outb(m_inp_out.inb(KB_CONTROL_PORT) & 0b00001110,
                   KB_CONTROL_PORT);
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
  #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  if (!_setmode(_fileno(stdin), _O_BINARY)) {
    throw "Failed to set stdin to binary mode";
  }
  #endif

  SQRPlayer player(std::cin);
  player.play();
  return 0;
}

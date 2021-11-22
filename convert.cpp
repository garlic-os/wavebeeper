#include <iostream>

#define INVALID_INPUT 1

using std::cin;
using std::cout;
using std::cerr;
using std::endl;


int main() {
  // Verify that the input is a valid WAV file.
  {
    char riff[4];
    char wave[4];
    cin.read(riff, 4);  // Now at offset 4
    cin.ignore(4);      // Now at offset 8
    cin.read(wave, 4);  // Now at offset 12
    if (strncmp(riff, "RIFF", 4) != 0 || strncmp(wave, "WAVE", 4) != 0) {
      cerr << "Invalid WAV file" << endl;
      exit(INVALID_INPUT);
    }
  }

  short num_channels;
  cin.ignore(10);  // Now at offset 22
  cin.read(reinterpret_cast<char*>(&num_channels), 2);  // Now at offset 24

  // Write the output file's header: "SQR\x00" followed by the sample rate.
  // The output's sample rate is the same as the input's.
  {
    char sample_rate[4];
    cout.write("SQR\x00", 4);
    cin.read(sample_rate, 4);  // Now at offset 28
    cout.write(sample_rate, 4);
  }

  short sample_length;  // Starts as bits per sample
  cin.ignore(6);  // Now at offset 34
  cin.read(reinterpret_cast<char*>(&sample_length), 2);  // Now at offset 36
  sample_length /= 8;  // Now bytes per sample

  cin.ignore(8);  // Now at offset 44: end of header, beginning of data
  char* sample = new char[sample_length];

  char bit_index = 0;
  char byte = 0;
  bool channel_state = false;

  // For each sample, output a 0 if the sample's value is below a certain
  // threshold and a 1 if it's at or above.
  // For 8-bit samples (which are unsigned), the threshold is 128.
  //   The most significant bit of the sample is unset below the threshold and
  //   vice versa.
  // For 16-bit samples and above (which are signed), the threshold is 0.
  //   The most significant bit of the sample (the negative bit) is set below
  //   the threshold and vice versa.
  while (cin.read(sample, sample_length)) {
    if (num_channels == 1 || channel_state) {
      // Set or unset the nth most sigificant bit in the byte based on the above
      // logic.
      byte |= ((sample[0] >> 7) != (sample_length > 1)) << (7 - bit_index);

      // Write the byte to the output every 8th bit processed.
      if (++bit_index == 8) {
        cout.write(&byte, 1);
        bit_index = 0;

        // Clear the byte for the next go-round (that big funny line can only
        // set bits, it can't unset them, so it only works when the byte starts
        // out as 0.)
        byte = 0;
      }
    }
    // Only process the left channel. In a stereo file, every other sample
    // belongs to the opposite channel.
    channel_state = !channel_state;
  }

  delete[] sample;
  return 0;
}

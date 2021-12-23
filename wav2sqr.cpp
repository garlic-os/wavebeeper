#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>

using std::cin;
using std::cout;
using std::endl;


inline unsigned long ceiling_divide(long a, long b) {
  return 1 + ((a - 1) / b);
}


int main(int argc, char *argv[]) {
  if (!_setmode(_fileno(stdin), _O_BINARY)) {
    throw "Failed to set stdin to binary mode";
  }
  if (!_setmode(_fileno(stdout), _O_BINARY)) {
    throw "Failed to set stdout to binary mode";
  }

  // Verify that the input is a valid WAV file.
  {
    char riff[4];
    char wave[4];
    cin.read(riff, 4);  // Now at offset 4
    cin.ignore(4);      // Now at offset 8
    cin.read(wave, 4);  // Now at offset 12
    if (strncmp(riff, "RIFF", 4) != 0 || strncmp(wave, "WAVE", 4) != 0) {
      throw "Invalid WAV file";
    }
  }

  unsigned short num_channels;
  cin.ignore(10);  // Now at offset 22
  cin.read(reinterpret_cast<char*>(&num_channels), 2);  // Now at offset 24

  // Write the SQR file signature to the output file.
  cout.write("SQR\x00", 4);

  // Copy the input's sample rate to the output.
  {
    char sample_rate[4];
    cin.read(sample_rate, 4);  // Now at offset 28
    cout.write(sample_rate, 4);
  }

  unsigned short bits_per_sample;
  cin.ignore(6);  // Now at offset 34
  cin.read(reinterpret_cast<char*>(&bits_per_sample), 2);  // Now at offset 36

  // Get and write the size of the data portion of the output file.
  // This number is the same as the number of samples in (one channel of) the
  // input file, since each sample will be reduced to one bit.
  // (Round up.)
  unsigned long data_size;
  cin.ignore(4);  // Now at offset 40
  cin.read(reinterpret_cast<char*>(&data_size), 4);  // Now at offset 44
  data_size = ceiling_divide(data_size, bits_per_sample);
  data_size = ceiling_divide(data_size, num_channels);
  cout.write(reinterpret_cast<char*>(&data_size), 4);

  // --- End of header, beginning of data ---
  char* sample = new char[bits_per_sample / 8];
  char bit_index = 0;
  char byte = 0;
  bool channel_state = argc > 1 && strncmp(argv[1], "-r", 2) == 0;  // "Right"

  // For each sample, output a 0 if the sample's value is below a certain
  // threshold and a 1 if it's at or above.
  // For 8-bit samples (which are unsigned), the threshold is 128.
  //   The most significant bit of the sample is unset below the threshold and
  //   vice versa.
  // For 16-bit samples and above (which are signed), the threshold is 0.
  //   The most significant bit of the sample (the negative bit) is set below
  //   the threshold and vice versa.
  while (cin.read(sample, bits_per_sample / 8)) {
    if (num_channels == 1 || channel_state) {
      // Set or unset the nth most sigificant bit in the byte based on the above
      // logic.
      byte |= ((sample[0] >> 7) != ((bits_per_sample / 8) > 1)) << (7 - bit_index);

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

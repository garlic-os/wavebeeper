#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>

#define INVALID_INPUT 1

using std::cin;
using std::cout;
using std::cerr;
using std::endl;


int main() {
  if (!_setmode(_fileno(stdin), _O_BINARY)) {
    throw "Failed to set stdin to binary mode";
  }
  if (!_setmode(_fileno(stdout), _O_BINARY)) {
    throw "Failed to set stdout to binary mode";
  }

  // Verify that the input is a valid SQR file.
  {
    char buffer[4];
    cin.read(buffer, 4);
    if (strncmp(buffer, "SQR\x00", 4) != 0) {
      cerr << "Invalid SQR file" << endl;
      exit(INVALID_INPUT);
    }
  }

  char sample_rate[4];
  cin.read(sample_rate, 4);

  // Each 1-bit long sample from the input file is expanded out to 8-bits,
  // making for 8 times the data size.
  unsigned long data_size;
  cin.read(reinterpret_cast<char*>(&data_size), 4);
  data_size *= 8;

  unsigned long file_size = data_size + 44;

  // Write the header for an 8-bit, mono WAV file with the same sample rate
  // as the input file, according to this guide:
  // https://docs.fileformat.com/audio/wav/
  cout << "RIFF";
  cout.write(reinterpret_cast<char*>(&file_size), 4);
  cout << "WAVEfmt ";
  cout.write("\x10\x00\x00\x00\x01\x00\x01\x00", 8);
  cout.write(sample_rate, 4);
  cout.write(sample_rate, 4);
  cout.write("\x01\x00\x08\x00", 4);
  cout << "data";
  cout.write(reinterpret_cast<char*>(&data_size), 4);

  char sample;
  while (cin.read(&sample, 1)) {
    // Iterate over each bit in the file from left to right.
    for (int i = 7; i >= 0; --i) {
      // If the bit is set, write a 0xFF to the output file.
      // Otherwise, write a 0x00.
      cout.put((sample & (1 << i)) ? '\xFF' : '\x00');
    }
  }

  return 0;
}

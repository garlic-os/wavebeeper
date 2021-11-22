#include <windows.h>  // HINSTANCE, PHANDLER_ROUTINE, WINAPI, __stdcall,
                      // LoadLibrary, FreeLibrary, GetProcAddress,
                      // SetConsoleCtrlHandler
#include <iostream>   // std::cin, std::cerr, std::endl
#include <thread>     // std::this_thread::sleep_for
#include <chrono>     // std::chrono::seconds

#define INVALID_INPUT 1
#define FAILED_DRIVER_LOAD 2
#define FAILED_DRIVER_OPEN 3
#define FAILED_REGISTER_CONTROL_HANDLER 4

using std::cin;
using std::cerr;
using std::endl;

typedef void  (__stdcall *lp_out)(short, short);
typedef short (__stdcall *lp_inp)(short);
typedef bool  (__stdcall *lp_driver_is_open)(void);

lp_out gfp_out;
lp_inp gfp_inp;


// Explanation:
// https://web.archive.org/web/20161030204856/https://courses.engr.illinois.edu/ece390/books/labmanual/io-devices-speaker.html
inline void beeper_on() {
  gfp_out(0x43, 0xB6);
}


inline void beep(const char period_low, const char period_high) {
  gfp_out(0x42, period_low);  // Set the low byte of the frequency.
  gfp_out(0x42, period_high);  // Set the high byte of the frequency.

  // Set bits 0 and 1 of the control register to start the note (and only
  // those two bits! There's important, unrelated stuff in the rest of this
  // register!)
  gfp_out(0x61, gfp_inp(0x61) | 0x03);
}


// Release the last beep (otherwise it holds forever)
inline void beeper_off() {
  // Reset bits 0 and 1 of the control register.
  gfp_out(0x61, gfp_inp(0x61) & 0xFC);
}


int main(int argc, char* argv[]) {
  // Verify that the input is a valid SQR file
  {
    char buffer[4];
    cin.read(buffer, 4);
    if (strncmp(buffer, "SQR\x00", 4) != 0) {
      cerr << "Invalid SQR file" << endl;
      exit(INVALID_INPUT);
    }
  }

  // Derive the length of note to play (in microseconds) from the file's sample
  // rate
  long sample_rate;  // per second
  cin.read(reinterpret_cast<char*>(&sample_rate), 4);
  short us_per_sample = round(1000000 / static_cast<double>(sample_rate));

  char sample_byte;
  char bit_index = 0;

  // @post: InpOut32.dll is loaded into dll_inpout. Do FreeLibrary() on it
  //           before exiting!
  //        Out32 is loaded into gfp_out();
  //        Inp32 is loaded into gfp_inp();
  //        on_kill() is set as the console Ctrl handler.
  HINSTANCE dll_inpout;
  {
    dll_inpout = LoadLibrary("InpOut32.DLL");
    if (dll_inpout == nullptr) {
      cerr << "Failed to load driver" << endl;
      exit(FAILED_DRIVER_LOAD);
    }

    gfp_out = (lp_out)GetProcAddress(dll_inpout, "Out32");
    gfp_inp = (lp_inp)GetProcAddress(dll_inpout, "Inp32");

    // Check that the driver was opened successfully
    if (!(lp_driver_is_open) GetProcAddress(dll_inpout, "IsInpOutDriverOpen")()) {
      cerr << "Failed to open driver" << endl;
      exit(FAILED_DRIVER_OPEN);
    }

    // If you don't have this the speaker will hang on its last note forever
    // if the program is ever killed.
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE) beeper_off, true)) {
      cerr << "Failed to set control handler" << endl;
      exit(FAILED_REGISTER_CONTROL_HANDLER);
    }
  }

  // For each sample, play the lowest note for the duration of the sample if it
  // is a 1, and play nothing for that time if it is a 0.
  while (cin.read(reinterpret_cast<char*>(&sample_byte), 1)) {
    if ((sample_byte >> (7 - bit_index)) & 1) {
      beeper_on();
      beep(0xFF, 0xFF);  // Lowest possible note
    } else {
      beeper_off();
    }
    bit_index = (bit_index + 1) % 8;

    // Play the pitch for 1/sample_rate seconds
    std::this_thread::sleep_for(std::chrono::microseconds(us_per_sample));
  }

  beeper_off();
  FreeLibrary(dll_inpout);
  return 0;
}

#include <iostream>
#include <stdexcept>
#include "windows.h"

using std::cout;
using std::endl;
using std::runtime_error;


class Beeper {
 private:
  HINSTANCE dll_inpout;

  typedef void  (__stdcall *lp_out)(short, short);
  typedef short (__stdcall *lp_inp)(short);
  typedef BOOL  (__stdcall *lp_driver_is_open)(void);

  lp_out gfp_out;
  lp_inp gfp_inp;

  static Beeper* instance;
  static BOOL WINAPI on_kill(DWORD signal) {
    instance->deactivate();
    return FALSE;
  }

  // Explanation: https://github.com/johnath/beep/blob/master/beep.c#L31
  inline int freq2period(const int freq) const {
    return static_cast<int>(
      static_cast<double>(1193180) / static_cast<double>(freq));
  }


 public:
  Beeper() {
    dll_inpout = LoadLibrary("InpOut32.DLL");
    if (dll_inpout == NULL) {
      throw runtime_error("Failed to load driver");
    }

    gfp_out = (lp_out)GetProcAddress(dll_inpout, "Out32");
    gfp_inp = (lp_inp)GetProcAddress(dll_inpout, "Inp32");

    // Check that the driver was opened successfully
    if (!(lp_driver_is_open) GetProcAddress(dll_inpout, "IsInpOutDriverOpen")()) {
      throw runtime_error("Failed to open driver");
    }

    // Awful static nonsense to register a kill signal handler from within a
    // class.
    // If you don't have this the speaker will hang on its last note forever
    // if the program is ever killed.
    Beeper::instance = this;
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE) on_kill, TRUE)) {
      throw runtime_error("Failed to set control handler");
    }
  }

  virtual ~Beeper() {
    deactivate();
    FreeLibrary(dll_inpout);
  }

  // Explanation:
  // https://web.archive.org/web/20161030204856/https://courses.engr.illinois.edu/ece390/books/labmanual/io-devices-speaker.html
  void activate() const {
    gfp_out(0x43, 0xB6);
  }

  void beep(const double freq) const {
    const int period = freq2period(freq);
    cout << period << endl;
    gfp_out(0x42, period & 0xFF);  // Set the low byte of the frequency.
    gfp_out(0x42, period >> 9);  // Set the high byte of the frequency.

    // Set bits 0 and 1 of the control register (and only those two bits!
    // There's important, unrelated stuff in the rest of this register!)
    gfp_out(0x61, gfp_inp(0x61) | 0x03);
  }

  // Release the last beep (otherwise it holds forever)
  void deactivate() const {
    // Reset bits 0 and 1 of the control register.
    gfp_out(0x61, gfp_inp(0x61) & 0xFC);
  }
};

Beeper* Beeper::instance = NULL;

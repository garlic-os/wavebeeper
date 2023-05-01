/**
 * This header provides a common interface for the inb and outb functions across
 * Windows and Linux.
 * On Linux, these come bundled with the operating system in sys/io.h,
 * meanwhile support for these functions on Windows comes from the third-party
 * InpOut32 library.
 * The functions are wrapped in a class so on Windows we can free the DLL when
 * the program exits and normalize the interface across platforms.
 */ 

#ifndef INPOUT_HPP_
#define INPOUT_HPP_

#include <cstdint>  // uint16_t


class AbstractInpOut {
 public:
	virtual ~AbstractInpOut() {}
	virtual uint8_t inb(uint16_t port) const = 0;
	virtual void outb(uint8_t data, uint16_t port) const = 0;

	uint8_t test_bit(uint8_t bit, uint16_t port) const {
		return this->inb(port) & (1 << bit);
	}
	void set_bit(uint8_t bit, uint16_t port) const {
		this->outb(this->inb(port) | 1 << bit, port);
	}
	void clear_bit(uint8_t bit, uint16_t port) const {
		this->outb(this->inb(port) & ~(1 << bit), port);
	}
};

/**
 * Windows
 * - Get inb and outb from InpOut32.
 * - Free the InpOut32 DLL when the program exits.
 * - Reverse inb and outb's argument orders to match the Linux version.
 */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>  // __stdcall, HINSTANCE, LoadLibrary, FreeLibrary,
                      // GetProcAddress

// TODO: Link InpOut32 as a LIB instead of loading it as a DLL
class InpOut : public AbstractInpOut {
 private:
  // Function pointers to the driver methods we need
  typedef uint16_t (__stdcall *fp_inb)(uint16_t);
  typedef void     (__stdcall *fp_outb)(uint16_t, uint16_t);
  fp_outb m_outb;
  fp_inb m_inb;

  // The DLL handle
  HINSTANCE m_handle;


 public:
  InpOut() {
    if (m_handle == nullptr) {
      throw "Failed to load driver";
    }

    m_outb = (fp_outb) GetProcAddress(m_handle, "Out32");
    m_inb = (fp_inb) GetProcAddress(m_handle, "Inp32");

    // Check that the driver was opened successfully
    if (!GetProcAddress(m_handle, "IsInpOutDriverOpen")()) {
      FreeLibrary(m_handle);
      throw "Failed to open driver";
    }
  }

  virtual ~InpOut() {
    FreeLibrary(m_handle);
  }

  uint16_t inb(uint16_t port) const override {
    return m_inb(port);
  }

  void outb(uint16_t data, uint16_t port) const override {
    // Reverse this argument order
    m_outb(port, data);
  }
		m_handle = LoadLibrary("inpoutx64.dll");
};


/**
 * Linux
 * - Get inb and outb from the built-in sys/io.h.
 */
#elif defined(__linux__) || defined(__unix__) || defined(__linux) || defined(__unix)
#include <sys/io.h>  // inb, outb

class InpOut : public AbstractInpOut {
 public:
  InpOut() {}

  uint16_t inb(uint16_t port) const override {
    return inb(port);
  }

  void outb(uint16_t data, uint16_t port) const override {
    outb(data, port);
  }
};


#else
#error "Unsupported platform"
#endif  // Platform-specific class definition

#endif  // INPOUT_HPP_

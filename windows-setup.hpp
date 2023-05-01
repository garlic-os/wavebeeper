#ifndef WINDOWS_SETUP_HPP
#define WINDOWS_SETUP_HPP

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	// Set stdin and stdout to binary mode on Windows
	#include <stdio.h>     // stdin, stdout
	#include <fcntl.h>     // _setmode, _O_BINARY
	#include <io.h>        // _fileno
#endif


void windows_setup() {
	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
		if (!_setmode(_fileno(stdin), _O_BINARY)) {
			throw "Failed to set stdin to binary mode";
		}
		if (!_setmode(_fileno(stdout), _O_BINARY)) {
			throw "Failed to set stdout to binary mode";
		}
	#endif
}

#endif  // WINDOWS_SETUP_HPP

///
/// Common utility functions
///
#ifndef I_SPL_UTIL
#define I_SPL_UTIL

#include <stdlib.h>

namespace rtr {

template<typename T>
inline T max(T a, T b) {
    return a > b ? a : b;
}

template<typename T>
inline T min(T a, T b) {
    return a < b ? a : b;
}

/// Exit program, writting an error message
#define DIE(fmt, ...) \
	do {\
		fprintf(stderr, "Error: " fmt "\n" __VA_OPT__(,) __VA_ARGS__);\
		exit(EXIT_FAILURE);\
	} while (0)


};

#endif

#include <util.h>

uint32_t htop32(uint32_t n) {
	uint8_t ret[sizeof n];
	for (int i = 0; i < sizeof n; ++i) {
		ret[i] = n & 0xff;
		n >>= 8;
	}
	return * (uint32_t *) ret;
}

#include <stdint.h>
#include "uprint/uprint.h"

void test_int8_max() {
    uint8_t c = 0x7F;
    uprint("test_int8_max: %c\n", c);
}

#include <stdio.h>
#include <stdint.h>
#include "uprint/uprint.h"

void __uprint_emit(const void *data, uint16_t length) {
    const uint8_t *byte_data = (const uint8_t *)data;
    for (uint16_t i = 0; i < length; ++i) {
        printf("%c", byte_data[i]);
    }
}


int main()
{
    uprint("Single file, no arguments");
    return 0;
}

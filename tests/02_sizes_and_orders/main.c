#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "uprint/uprint.h"

void __uprint_emit(const void *data, uint16_t length) {
    const uint8_t *byte_data = (const uint8_t *)data;
    for (uint16_t i = 0; i < length; ++i) {
        printf("%c", byte_data[i]);
    }
}

/*******************************************************************************************
 *      1 byte length variable test
*******************************************************************************************/

void test_char_A() {
    char c = 'A';
    uprint("test_char_A: %c", c);
}

void test_uint8_min() {
    uint8_t c = 0;
    uprint("test_uint8_min: %c", c);
}

void test_uint8_max() {
    uint8_t c = 0xFF;
    uprint("test_uint8_max: %c", c);
}

void test_int8_min() {
    uint8_t c = 0x80;
    uprint("test_int8_min: %c", c);
}

void test_int8_max() {
    uint8_t c = 0x7F;
    uprint("test_int8_max: %c", c);
}

/*******************************************************************************************
 *      2 bytes length variable test
*******************************************************************************************/

void test_short_min() {
    short num = -32768;
    uprint("test_short_min: %d", num);
}

void test_short_max() {
    short num = 0x7FFF;
    uprint("test_short_max: %d", num);
}

void test_short_1234() {
    short num = 0x3039;
    uprint("test_short_1234: %d", num);
}

/*******************************************************************************************
 *      4 bytes length variable test
*******************************************************************************************/

void test_int32_min() {
    int32_t num = 0x80000000;
    uprint("test_int32_min: %d", num);
}

void test_int32_max() {
    int32_t num = 0x7FFFFFFF;
    uprint("test_int32_max: %d", num);
}

void test_int32_1234567890() {
    int32_t num = 0x499602D2;
    uprint("test_int32_1234567890: %d", num);
}

void test_uint32_min() {
    uint32_t num = 0;
    uprint("test_uint32_min: %d", num);
}

void test_uint32_max() {
    uint32_t num = 0xFFFFFFFF;
    uprint("test_uint32_max: %d", num);
}

void test_uint32_1234567890() {
    uint32_t num = 0x499602D2;
    uprint("test_uint32_1234567890: %d", num);
}

/*******************************************************************************************
 *      4 bytes float variable test
*******************************************************************************************/

void test_float_zero() {
    float num = 0.0f;
    uprint("test_float_zero: %f", num);
}

void test_float_pi() {
    float num = 3.14159265f;
    uprint("test_float_pi: %f", num);
}

void test_float_neg() {
    float num = -2.5f;
    uprint("test_float_neg: %f", num);
}

/*******************************************************************************************
 *      8 bytes double variable test
*******************************************************************************************/

void test_double_zero() {
    double num = 0.0;
    uprint("test_double_zero: %f", num);
}

void test_double_pi() {
    double num = 3.141592653589793;
    uprint("test_double_pi: %f", num);
}

void test_double_neg() {
    double num = -2.5;
    uprint("test_double_neg: %f", num);
}

/*******************************************************************************************
 *      8 bytes length variable test
*******************************************************************************************/

void test_int64_min() {
    int64_t num = 0x8000000000000000;
    uprint("test_int64_min: %ld", num);
}

void test_int64_max() {
    int64_t num = 0x7FFFFFFFFFFFFFFF;
    uprint("test_int64_max: %ld", num);
}

void test_int64_1to0to1() {
    int64_t num = 0xAB54A98EEE391EEA;
    uprint("test_int64_1to0to1: %ld", num);
}

void test_uint64_min() {
    uint64_t num = 0;
    uprint("test_uint64_min: %ld", num);
}

void test_uint64_max() {
    uint64_t num = 0xFFFFFFFFFFFFFFFF;
    uprint("test_uint64_max: %ld", num);
}

void test_uint64_1to0to1() {
    uint64_t num = 0xAB54A98EEE391EEA;
    uprint("test_uint64_1to0to1: %ld", num);
}

// C Promotion test
void test_promotion() {
    uprint("test_promotion: %d", 0);
}

int main()
{
    test_char_A();
    test_uint8_min();
    test_uint8_max();
    test_int8_min();
    test_int8_max();

    test_short_min();
    test_short_max();
    test_short_1234();

    test_int32_min();
    test_int32_max();
    test_int32_1234567890();
    test_uint32_min();
    test_uint32_max();
    test_uint32_1234567890();

    test_float_zero();
    test_float_pi();
    test_float_neg();

    test_double_zero();
    test_double_pi();
    test_double_neg();

    test_int64_min();
    test_int64_max();
    test_int64_1to0to1();
    test_uint64_min();
    test_uint64_max();
    test_uint64_1to0to1();

    test_promotion();

    return 0;
}
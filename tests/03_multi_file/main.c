#include "uprint/uprint.h"
#include "third.h"

void test_literal() {
    uprint("literal-only");
}

int main()
{
    test_literal();

    test_int8_max();

    return 0;
}

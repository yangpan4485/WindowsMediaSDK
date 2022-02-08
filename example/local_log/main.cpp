#include <iostream>

#include "local_log.h"

int main(void) {
    LOGI("Hello") << 123;
    LOGW("Hello") << 123;
    LOGE("Hello") << 123;
    getchar();
    return 0;
}

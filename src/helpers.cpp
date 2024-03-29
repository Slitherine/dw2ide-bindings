#pragma once
#include "helpers.h"

int CheckedNarrowToInt(size_t v) {
    if (v > INT32_MAX) {
        DEBUG_BREAK();
        return INT32_MAX;
    }
    return static_cast<int>(v);
}
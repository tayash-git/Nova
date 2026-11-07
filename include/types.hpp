#pragma once

#include <cstdint>

// verify different datatypes at compile time
static_assert(sizeof(int) == 4, "int datatype is not 32 bits!");
static_assert(sizeof(uint64_t) == 8, "long long datatype is not 64 bits!");

namespace types
{
    using QWORD =               uint64_t;
}

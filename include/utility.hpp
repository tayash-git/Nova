#pragma once

#include "types.hpp"
using namespace types;

// utility functions

namespace utility
{
    inline constexpr QWORD align_up(QWORD n, QWORD alignment)
    {
        return ((n + alignment - 1) / alignment) * alignment;
    }
}

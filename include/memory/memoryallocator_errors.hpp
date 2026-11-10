#pragma once

namespace memoryallocator {
    enum class MemoryError {
        BadAllocation,
        BadDelete,
        OutOfMemory,
        BadAccess,
        OutOfBounds
    };
}

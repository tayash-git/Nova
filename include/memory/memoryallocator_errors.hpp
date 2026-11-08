#pragma once

namespace memoryallocator {
    enum class MemoryAllocationError {
        BadAllocation,
        BadDelete,
        OutOfMemory
    };
}

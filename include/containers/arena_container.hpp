#pragma once

#include "../memory/memoryallocator.hpp"
#include <expected>
#include <malloc.h>
#include <memory>

namespace containers {
    template <typename T>
    class ArenaVector {
    private:
        std::unique_ptr<memoryallocator::ArenaAllocator> allocator;

    public:
        ArenaVector() {
            allocator = std::make_unique<memoryallocator::ArenaAllocator>();
        }

        template <typename... Args>
        std::expected<QWORD, memoryallocator::MemoryAllocationError> push_back(T resource, Args&&... args) {
            auto allocated_memory = allocator->construct_resource<T>(1, std::forward<Args>(args)...);

            if (!allocated_memory)
                return std::unexpected(allocated_memory.error());

            return allocated_memory.value();
        }

        T operator[](QWORD i) {
             // implement indexing from the allocator to return a value
        }
    };
}

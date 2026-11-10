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
        T* base = nullptr;              // where the first element lives
        size_t count = 0;               // amount of elements

    public:
        ArenaVector() {
            allocator = std::make_unique<memoryallocator::ArenaAllocator>();
        }

        template <typename... Args>
        std::expected<QWORD, memoryallocator::MemoryError> push_back(T resource, Args&&... args) {
            auto allocated_memory = allocator->construct_resource<T>(1, std::forward<Args>(args)...);

            if (!allocated_memory)
                return std::unexpected(allocated_memory.error());

            return allocated_memory.value();
        }

        std::expected<T, memoryallocator::MemoryError> operator[](QWORD i) {
             if (i < 0 || i > count - 1) [[unlikely]]
                 return std::unexpected(memoryallocator::MemoryError::OutOfBounds);

             return base[i];
        }
    };
}

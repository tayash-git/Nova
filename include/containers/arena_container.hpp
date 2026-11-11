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

    public:
        size_t count = 0;               // amount of elements
        ArenaVector()
            : allocator(std::make_unique<memoryallocator::ArenaAllocator>()) {}

        ArenaVector(ArenaVector&& move_vector) {
            if (!move_vector.allocator)
                return;

            allocator = std::move(move_vector.allocator);
            base = move_vector.base;
            count = move_vector.base;
        }

        // constructs a resource and returns an index to it
        template <typename... Args>
        std::expected<uint64_t, memoryallocator::MemoryError> push_back(T resource, Args&&... args) {
            auto allocated_memory = allocator->construct_resource<T>(1, std::forward<Args>(args)...);

            if (!allocated_memory)
                return std::unexpected(allocated_memory.error());

            if (allocated_memory.value() == nullptr)
                return std::unexpected(memoryallocator::MemoryError::BadAllocation);

            uint64_t index = reinterpret_cast<uint64_t>(allocated_memory.value()) - reinterpret_cast<uint64_t>(base);
            return index;
        }

        // [] operator overload to index the vector
        std::expected<T, memoryallocator::MemoryError> operator[](QWORD i) {
             if (i < 0 || i > count - 1) [[unlikely]]
                 return std::unexpected(memoryallocator::MemoryError::OutOfBounds);

             return base[i];
        }
    };
}

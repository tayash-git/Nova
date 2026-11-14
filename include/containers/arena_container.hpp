#pragma once

#include "../memory/memoryallocator.hpp"
#include <cassert>
#include <expected>
#include <functional>
#include <malloc.h>
#include <memory>
#include <utility>

namespace containers {
    template <typename T>
    class ArenaVector {
    private:
        std::unique_ptr<memoryallocator::ArenaAllocator> allocator;
        T* base = nullptr;              // where the first element lives

    public:
        size_t count = 0;               // amount of elements
        ArenaVector()
            : allocator(std::make_unique<memoryallocator::ArenaAllocator>()),
            base(reinterpret_cast<T*>(allocator->get_pointer())) {}

        ArenaVector(ArenaVector&) = delete;

        ArenaVector(ArenaVector&& move_vector) 
            : allocator(std::move(move_vector.allocator)), base(move_vector.base), count(move_vector.count) {
            base = nullptr;
        }

        // copies a resource into the vector
        std::expected<uint64_t, memoryallocator::MemoryError> push_back(T& resource) noexcept {
            auto allocated_memory = allocator->allocate<T>(1);

            if (!allocated_memory)
                return std::unexpected(allocated_memory.error());

            T* copied_object = new (allocated_memory) T(resource);

            uint64_t index = reinterpret_cast<uint64_t>(allocated_memory.value()) - reinterpret_cast<uint64_t>(base);
            return index;
        }

        // copies a resource into the vector
        std::expected<uint64_t, memoryallocator::MemoryError> push_back(T&& resource) noexcept {
            auto allocated_memory = allocator->allocate<T>(1);

            if (!allocated_memory)
                return std::unexpected(allocated_memory.error());

            T* target = new (allocated_memory) T(resource);

            uint64_t index = reinterpret_cast<uint64_t>(allocated_memory.value()) - reinterpret_cast<uint64_t>(base);
            return index;
        }

        // unsafe
        std::expected<void, memoryallocator::MemoryError> insert_at_end(T* array, size_t amount) noexcept {
            auto allocated_memory = allocator->allocate<T>(amount);

            if (!allocated_memory)
                return std::unexpected(allocated_memory.error());

            memcpy(allocated_memory.value(), array, amount);
        }

        // --------------------------------------------------------------------------------------------------------------------------------

        // [] operator overload to index the vector
        T operator[](size_t i) noexcept {
            assert(i < count);
            [[assume(i < count)]];

            return base[i];
        }

        // [] operator overload to index the vector
        T operator[](size_t i) const noexcept {
            assert(i < count);
            [[assume(i < count)]];

            return base[i];
        }

        // move equal expression
        ArenaVector& operator=(ArenaVector&& vector_target) {
            if (&vector_target == this)
                return *this;

            allocator = std::move(vector_target.allocator);
            base = vector_target.base;
            count = vector_target.count;

            return *this;
        }

        [[gnu::pure]] T* begin() { return base; };
        [[gnu::pure]] T* end() { return base + count; }
        [[gnu::pure]] const T* begin() const { return base; }
        [[gnu::pure]] const T* end() const { return base + count; }
    };
}

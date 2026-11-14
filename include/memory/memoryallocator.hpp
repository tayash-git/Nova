#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory.h>
#include <memory>
#include <memoryapi.h>
#include <expected>
#include <type_traits>
#include <winnt.h>

#include "../config/config.hpp"
#include "memoryallocator_errors.hpp"
#include "memoryallocator_object_tracker.hpp"

// Switching to using a shared pointer instead of a raw uintptr_t for the base memory (this is for copy constructors)

using namespace config;

namespace memoryallocator {
    class IAllocator {
    protected:
        virtual LPVOID  alloc(size_t bytes, short alignment) = 0;
        virtual void    dealloc(LPVOID pointer) = 0;
    public:
        virtual ~IAllocator() = default;
    };

    class ArenaAllocator : public IAllocator {
    public:
        ArenaAllocator();

        ArenaAllocator(ArenaAllocator& copy_allocator);
        ArenaAllocator(ArenaAllocator&& moved_allocator);
        ~ArenaAllocator() override;

        // allocates memory without constructing an object
        template<typename T, size_t count = 1>
        [[using gnu: malloc, hot]] T* allocate() noexcept {
            static_assert(count > 0, "count must be greater than 0");
            [[assume(count > 0)]];

            short alignment = alignof(T);
            size_t byte_count = sizeof(T) * count;
            auto allocated_memory = alloc(byte_count, alignment);

            if (!allocated_memory) [[unlikely]]
                return nullptr;
            else [[likely]]
                return allocated_memory;
        }

        // allocates memory and constructs 'Count' objects (accepts constructor arguments)
        template <typename T, size_t count = 1, typename... Args>
        [[gnu::malloc]] T* construct_resource(Args&&... args) noexcept {
            static_assert(count > 0, "count must be greater than 0");
            [[assume(count > 0)]];

            T* allocated_memory = allocate<T>(count);

            if (!allocated_memory) [[unlikely]]
                return nullptr;

            T* constructed_address = reinterpret_cast<T*>(allocated_memory);

            for (auto i{0uz}; i < count; ++i)
                new (constructed_address + i) T(std::forward<Args>(args)...);

            // check if the allocated object is not trivially destructible
            if constexpr (!std::is_trivially_destructible_v<T>) {
                DestructorNodeBase* node_allocated = allocate<DestructorNode<T>>(1);

                if (!node_allocated) [[unlikely]]
                    return nullptr;

                DestructorNodeBase* node = new (node_allocated) DestructorNode<T>(constructed_address, count);
                node->previous = recent_obj_node;

                recent_obj_node = node;
            }

            return constructed_address;
        }

        [[maybe_unused]] [[using gnu: always_inline, pure]] inline const uintptr_t get_pointer() const { return memory; }

    private:
        uintptr_t memory;           // the address where the reserve starts
        uintptr_t free_slot;        // pointer to the next free slot
        size_t capacity;            // how many bytes fit in the current committed amount
        size_t size;                // how many bytes were allocated
        std::shared_ptr<int8_t> ref_count;                        // how many references to the allocator (used for destructors)
        DestructorNodeBase* recent_obj_node = nullptr;          // most recent node holding allocated objects

        [[using gnu: malloc, hot]] LPVOID alloc(size_t bytes, short alignment) noexcept override;

        std::expected<void, MemoryError> resize_memory() noexcept;

        [[using gnu: always_inline, pure]] bool enough_memory_for_commit(uintptr_t address, size_t size) const noexcept {        // checks if enough memory exists in the committed amount
            return address + size < memory + capacity;
        }

        [[gnu::pure]] bool enough_memory_for_reserve() const noexcept {
            if (capacity * 2 > ALLOCATOR_RESERVE_SIZE) [[unlikely]]
                return false;
            else [[likely]]
                return true;
        }

        void dealloc(LPVOID ptr) override {}
    };
}

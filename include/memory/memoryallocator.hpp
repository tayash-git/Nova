#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory.h>
#include <memory>
#include <memoryapi.h>
#include <expected>
#include <stdexcept>
#include <type_traits>
#include <winnt.h>

#include "../utility.hpp"
#include "../config/config.hpp"
#include "memoryallocator_errors.hpp"
#include "memoryallocator_object_tracker.hpp"

// Switching to using a shared pointer instead of a raw uintptr_t for the base memory (this is for copy constructors)

using namespace config;

namespace memoryallocator {
    class IAllocator {
    protected:
        virtual std::expected<LPVOID, MemoryError> alloc(size_t bytes, short alignment) = 0;
        virtual void dealloc(LPVOID pointer) = 0;
    public:
        virtual ~IAllocator() = default;
    };

    class ArenaAllocator : public IAllocator {
    public:
        ArenaAllocator()
            : capacity(ALLOCATOR_COMMIT_SIZE) {
            // reserves a huge amount of contigous memory totaling 16GB
            memory = reinterpret_cast<uintptr_t>(VirtualAlloc(
                NULL,
                ALLOCATOR_RESERVE_SIZE,
                MEM_RESERVE,
                PAGE_NOACCESS
            ));

            if (!memory) [[unlikely]]
                throw std::runtime_error("bad reserve in ArenaAllocator : IAllocator constructor\n");

            auto ok = VirtualAlloc(
                reinterpret_cast<LPVOID>(memory),
                ALLOCATOR_COMMIT_SIZE,
                MEM_COMMIT,
                PAGE_READWRITE
            );

            if (!ok) [[unlikely]]
                throw std::runtime_error("bad commit in ArenaAllocator : IAllocator constructor\n");

            free_slot = memory;
        }

        ArenaAllocator(ArenaAllocator& copy_allocator) = delete; // prevent copying due to unique ownership

        ArenaAllocator(ArenaAllocator&& move_allocator)
            : memory(move_allocator.memory), free_slot(move_allocator.free_slot), capacity(move_allocator.capacity),
            size(move_allocator.size), recent_obj_node(move_allocator.recent_obj_node) {
                move_allocator.memory = NULL;
                move_allocator.free_slot = NULL;
                move_allocator.capacity = 0;
                move_allocator.size = 0;
                move_allocator.recent_obj_node = nullptr;
            }

        ~ArenaAllocator() override {
        // delete all allocated trivially destructible objects
            while (recent_obj_node != nullptr) {
                DestructorNodeBase* temporary_previous = recent_obj_node->previous;
                recent_obj_node->~DestructorNodeBase();
                recent_obj_node = temporary_previous;
            }

            if (memory == NULL)
                return;

            VirtualFree(
                reinterpret_cast<LPVOID>(memory),
                0,
                MEM_RELEASE
            );
        }

        template<typename T>
        std::expected<T*, MemoryError> allocate(size_t amount) noexcept {
            short alignment = alignof(T);
            size_t byte_count = sizeof(T) * amount;
            auto allocated_memory = alloc(byte_count, alignment);

            if (!allocated_memory) [[unlikely]]
                return std::unexpected(allocated_memory.error());
            else [[likely]]
                return allocated_memory.value();
        }

        // allocates memory and constructs 'Count' objects (accepts constructor arguments)
        template <typename T, typename... Args>
        std::expected<T*, MemoryError> construct_resource(size_t count, Args&&... args) noexcept {
            if (count < 1) [[unlikely]]
                return std::unexpected(MemoryError::BadAllocation);

            auto allocated_memory = allocate<T>(count);

            if (!allocated_memory) [[unlikely]]
                return std::unexpected(allocated_memory.error());

            T* constructed_address = reinterpret_cast<T*>(allocated_memory.value());

            for (auto i{0uz}; i < count; ++i) {
                new (constructed_address + i) T(std::forward<Args>(args)...);
            }

            // check if the allocated object is not trivially destructible
            if constexpr (!std::is_trivially_destructible_v<T>) {
                auto node_allocated = allocate<DestructorNode<T>>(1);

                if (!node_allocated) [[unlikely]]
                    return std::unexpected(node_allocated.error());

                DestructorNodeBase* node_pointer = reinterpret_cast<DestructorNodeBase*>(node_allocated.value());
                DestructorNodeBase* node = new (node_pointer) DestructorNode<T>(constructed_address, count);
                node->previous = recent_obj_node;

                recent_obj_node = node;
            }

            return constructed_address;
        }

        const void* get_pointer() const { return memory.get(); }

    private:
        std::shared_ptr<void> memory;           // the address where the reserve starts
        uintptr_t free_slot;        // pointer to the next free slot
        size_t capacity;            // how many bytes fit in the current committed amount
        size_t size;                // how many bytes were allocated
        DestructorNodeBase* recent_obj_node = nullptr;        // most recent node holding an allocated object

        std::expected<LPVOID, MemoryError> alloc(size_t bytes, short alignment) noexcept override {
            uintptr_t free_slot_aligned = utility::align_up(free_slot, alignment);

            // attempt to resize memory if there's not enough
            while (!enough_memory_for_commit(free_slot_aligned, bytes)) [[unlikely]] {
                if (auto ok = resize_memory(); !ok) [[unlikely]]
                    return std::unexpected(ok.error());
            }

            free_slot = free_slot_aligned + bytes;

            return reinterpret_cast<LPVOID>(free_slot_aligned);
        }

        // commits 'capacity' bytes and doubles capacity
        std::expected<void, MemoryError> resize_memory() noexcept {
            if (!enough_memory_for_reserve()) [[unlikely]]
                return std::unexpected(MemoryError::OutOfMemory);

            LPVOID ok = VirtualAlloc(
                reinterpret_cast<LPVOID>(memory.get() + capacity),
                capacity,
                MEM_COMMIT,
                PAGE_READWRITE
            );

            if (!ok) [[unlikely]]
                return std::unexpected(MemoryError::BadAllocation);

            capacity *= 2;

            return{};
        }

        bool enough_memory_for_commit(uintptr_t address, size_t size) const noexcept {        // checks if enough memory exists in the committed amount
            return address + size < memory + capacity;
        }

        bool enough_memory_for_reserve() const noexcept {       // checks if theres enough memory reserve
            if (capacity * 2 > ALLOCATOR_RESERVE_SIZE) [[unlikely]]
                return false;
            else [[likely]]
                return true;
        }

        void dealloc(LPVOID ptr) override {}
    };
}

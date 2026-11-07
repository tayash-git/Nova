#pragma once

#include <cmath>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory.h>
#include <memory>
#include <memoryapi.h>
#include <new>
#include <expected>
#include <winnt.h>

#include "../config/config.hpp"
#include "memoryallocator_errors.hpp"

// Switching to VirtualAlloc later on to allow for committing and reserving memory giving more control.

using namespace config;

namespace memoryallocator {
    template <typename T>
    class IAllocator {
        virtual std::expected<T*, MemoryAllocationError> alloc() = 0;
        virtual ~IAllocator() = default;
    };

    template <typename T>
    class ArenaAllocator : public IAllocator<T> {
    public:
        explicit ArenaAllocator() {
            memory = VirtualAlloc(
                NULL,
                ALLOCATOR_RESERVE_SIZE,
                MEM_RESERVE,
                PAGE_NOACCESS
            );

            VirtualAlloc(
                memory,
                ALLOCATOR_COMMIT_SIZE,
                MEM_COMMIT,
                PAGE_READWRITE
            );
        }

        std::expected<T*, MemoryAllocationError> alloc(size_t count) override {
            if (count + amount >= allocated)
                resize_memory();

            // implement virtual alloc commit
        }

    private:
        T* memory;
        T* current_slot;     // pointer to the current free slot
        size_t allocated;   // how many elements are allocated
        size_t amount;      // how many elements exist in the arena

        ~ArenaAllocator() {
            try {
                delete[] memory;
            } catch (const std::bad_alloc& e) {
                std::cerr << e.what();
                throw;
            }
        }

        void resize_memory() {
            // use virtualalloc to commit more memory
        }
    };
}

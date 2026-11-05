#pragma once

#include <cmath>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory.h>
#include <memory>
#include <new>
#include <expected>

namespace memory_allocator {
    template <typename T>
    class IAllocator {
        virtual T* alloc() = 0;
        virtual void dealloc() = 0;
        virtual ~IAllocator() = default;
    };

    template <typename T>
    class ArenaAllocator : public IAllocator<T> {
    public:
        explicit ArenaAllocator(size_t element_count, size_t rescale)
            : rescale(rescale), allocated(element_count) {
            try {
                memory = new T[element_count];
            } catch (const std::bad_alloc& e) {
                std::cerr << e.what() << " : bad alloc in ArenaAllocator : IAllocator";
                throw;
            }
        }

        std::expected<T*, const std::bad_alloc&> alloc(size_t count) override {
            if (count + amount >= allocated)
                resize_memory();

            try {
                T* object = new(current_slot) T[count]; // allocate at the next free slot
                amount++;
                return object;
            } catch (const std::bad_alloc& e) {
                std::cerr << e.what();
                return std::unexpected(e);
            }
        }

    private:
        T* memory;
        T* current_slot;     // pointer to the current free slot
        size_t allocated;   // how many elements are allocated
        size_t amount;      // how many elements exist in the arena
        const size_t rescale;     // the rate at which memory grows in the arena

        ~ArenaAllocator() {
            try {
                delete[] memory;
            } catch (const std::bad_alloc& e) {
                std::cerr << e.what();
                throw;
            }
        }

        void resize_memory() {
            // move memory to a temporary location then reallocate to increase size and copy into the new location
            std::unique_ptr<T> temporary_memory = std::make_unique<T>(amount);

            std::memcpy(
                temporary_memory.get(),
                memory,
                amount * sizeof(T)
            );

            delete[] memory;
            memory = new T[std::floor(allocated * rescale)];

            allocated = std::floor(allocated * rescale);

            std::memcpy(
                memory,
                temporary_memory.get(),
                amount * sizeof(T)
            );
        }
    };
}

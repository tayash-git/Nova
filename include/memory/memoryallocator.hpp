#pragma once

#include <cmath>
#include <cstddef>
#include <cstring>
#include <memory.h>
#include <memory>
#include <expected>
#include <system_error>

namespace memory_allocator
{
    class IAllocator
    {
        virtual void* alloc() = 0;
        virtual void dealloc() = 0;
        virtual ~IAllocator() = default;
    };

    template <typename T>
    class ArenaAllocator : public IAllocator
    {
        private:
            T* memory_pool;
            T* pointer;                     // pointer to the next free memory
            std::size_t memory_capacity;    // total memory allocated in bytes
            std::size_t memory_size;        // how much data is stored in bytes
            std::size_t growth_multiplier;  // how much to grow the memory pool

        public:
            ArenaAllocator(std::size_t reserve = sizeof(T), std::size_t growth = 1)
                : memory_pool(new T[reserve]), growth_multiplier(growth), pointer(memory_pool) { }

            ~ArenaAllocator() { delete[] memory_pool; }
    
            /*
             * allocates memory to the memory pool.
             *
             * the way it works is that it holds a very large
             * memory pool to allocate memory super quickly to.
             * 
             * if the pool runs out of memory then it reallocates to a larger
             * size.
            */
            T* alloc(std::size_t elements = sizeof(T)) override
            {
                if (memory_size + elements > memory_capacity - sizeof(T)) [[unlikely]]
                {
                    std::unique_ptr<T> temporary = std::make_unique<T>(memory_size);
                    std::memcpy(
                        temporary,
                        memory_pool,
                        memory_size
                    );

                    // whenever we run out of memory we allocate more according to the following formula
                    // newsize = (oldsize + insertedelementssize) * growth multiplier
                    // and we square the growth multiplier.
                    delete[] memory_pool;
                    memory_pool = new T[(memory_capacity + elements) * growth_multiplier];
                    growth_multiplier = std::pow(growth_multiplier, 2);

                    std::memcpy(
                        memory_pool,
                        temporary,
                        memory_size
                    );
                }

                T* allocated = new (pointer) T[elements];
                return allocated;
            }

            std::expected<void, std::error_code> dealloc(T* ptr) override
            {
                if (ptr < memory_pool || ptr > memory_pool + memory_capacity)
                    return std::unexpected(std::make_error_code(std::errc::address_not_available));

                delete[] ptr;
            }
    };
}

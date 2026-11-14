#include "../../include/utility.hpp"
#include "../../include/memory/memoryallocator.hpp"

namespace memoryallocator {
    ArenaAllocator::ArenaAllocator()
        : capacity(ALLOCATOR_COMMIT_SIZE), ref_count(std::make_shared<int8_t>(0)) {
        // reserves a huge amount of contigous memory totaling 16GB
        memory = reinterpret_cast<uintptr_t>(VirtualAlloc(
            NULL,
            ALLOCATOR_RESERVE_SIZE,
            MEM_RESERVE,
            PAGE_NOACCESS
        ));

        if (!memory) [[unlikely]]
            throw std::runtime_error("bad reserve in ArenaAllocator : IAllocator constructor\n");

        LPVOID ok = VirtualAlloc(
            reinterpret_cast<LPVOID>(memory),
            ALLOCATOR_COMMIT_SIZE,
            MEM_COMMIT,
            PAGE_READWRITE
        );

        if (!ok) [[unlikely]]
            throw std::runtime_error("bad commit in ArenaAllocator : IAllocator constructor\n");

        free_slot = memory;
    }

    // copies values and increases the reference counter
    ArenaAllocator::ArenaAllocator(ArenaAllocator& copy_allocator)
        : memory(copy_allocator.memory), free_slot(copy_allocator.free_slot),
        size(copy_allocator.size), capacity(copy_allocator.capacity), ref_count(copy_allocator.ref_count) {
        ++(*ref_count);
    }

    ArenaAllocator::ArenaAllocator(ArenaAllocator&& moved_allocator)
        : memory(moved_allocator.memory),
        free_slot(moved_allocator.free_slot),
        size(moved_allocator.size),
        capacity(moved_allocator.capacity),
        ref_count(moved_allocator.ref_count) {
        ++(*ref_count);
    }

    // release all resources if there are no more references
    ArenaAllocator::~ArenaAllocator() {
        if (--(*ref_count) > 0)
            return;

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

    [[using gnu: malloc, hot]] LPVOID ArenaAllocator::alloc(size_t bytes, short alignment) noexcept {
        uintptr_t free_slot_aligned = utility::align_up(free_slot, alignment);

        // attempt to resize memory if there's not enough
        while (!enough_memory_for_commit(free_slot_aligned, bytes)) [[unlikely]] {
            if (auto ok = resize_memory(); !ok) [[unlikely]]
                return nullptr;
        }

        free_slot = free_slot_aligned + bytes;

        return reinterpret_cast<LPVOID>(free_slot_aligned);
    }

    // commits 'capacity' bytes and doubles capacity
    std::expected<void, MemoryError> ArenaAllocator::resize_memory() noexcept {
        if (!enough_memory_for_reserve()) [[unlikely]]
            return std::unexpected(MemoryError::OutOfMemory);

        LPVOID ok = VirtualAlloc(
            reinterpret_cast<LPVOID>(memory + capacity),
            capacity,
            MEM_COMMIT,
            PAGE_READWRITE
        );

        if (!ok) [[unlikely]]
            return std::unexpected(MemoryError::BadAllocation);

        capacity *= 2;

        return{};
    }
}

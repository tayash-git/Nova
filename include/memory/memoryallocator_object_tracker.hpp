#pragma once

#include <cstddef>
namespace memoryallocator {
    struct DestructorNodeBase {
        DestructorNodeBase* previous = nullptr;
        virtual ~DestructorNodeBase() = default;
    };

    // a chain of nodes that link each other and each hold a pointer to a dynamically allocated object.
    template <typename T>
    struct DestructorNode final : public DestructorNodeBase {
        T* object = nullptr;
        size_t count = 0;

        DestructorNode(T* object, size_t count)
            : object(object), count(count) {}

        // calls the destructor of the object then calls the destructor of the previous node
        ~DestructorNode() override {
            if (object == nullptr) [[unlikely]]
                return;

            for (auto i{count}; i-- > 0;) {
                (object + i)->~T();
            }
        }
    };
}

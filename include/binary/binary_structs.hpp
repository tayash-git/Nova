#pragma once

#include "../containers/arena_container.hpp"

namespace binary_structs {
    enum class SectionNames {
        Text,
        Data,
        Rdata,
        Bss
    };

    struct section_structure {
        std::string name;
        containers::ArenaVector<uint8_t> data;
    };
}

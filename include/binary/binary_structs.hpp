#pragma once

#include "../containers/arena_container.hpp"
#include <minwindef.h>
#include <winnt.h>
#include <vector>

namespace binary_structs {
    enum class SectionNames {
        Text,
        Data,
        Rdata,
        Bss
    };

    struct SectionStructure {
        std::string name;
        containers::ArenaVector<uint8_t> data;
        containers::ArenaVector<IMAGE_RELOCATION> relocations;
        DWORD characteristics;
    };

    struct ObjectFile {
        containers::ArenaVector<SectionStructure> sections;
        containers::ArenaVector<IMAGE_SYMBOL> symbol_table;
        std::vector<unsigned char> string_table;

        explicit ObjectFile(ObjectFile&& object_to_move)
            : sections(std::move(object_to_move.sections)),
            symbol_table(std::move(object_to_move.symbol_table)),
            string_table(std::move(object_to_move.string_table)) {}
    };
}

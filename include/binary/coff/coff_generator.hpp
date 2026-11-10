#pragma once

#include <memory>
#include <vector>
#include <cstdint>
#include <windows.h>
#include <winnt.h>

#include "../binary_structs.hpp"
#include "../../containers/arena_container.hpp"

namespace coff_generator {
    std::unique_ptr<std::vector<uint8_t>> generate_coff_file(
        containers::ArenaVector<binary_structs::section_structure>& sections_map,       // contains the sections
        containers::ArenaVector<IMAGE_RELOCATION>& relocation_table,                    // relocation table
        containers::ArenaVector<IMAGE_SYMBOL>& symbol_table,                            // symbol table
        containers::ArenaVector<unsigned char>& string_table                             // strings for symbol names
    );
}

#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <cstdint>

#include "structs.hpp"

namespace coff_generator {
    std::unique_ptr<std::vector<uint8_t>> generate_coff_file(
        std::unordered_map<SectionNames, std::vector<uint8_t>> sections_map
    );
}

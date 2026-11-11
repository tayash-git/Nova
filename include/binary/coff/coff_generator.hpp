#pragma once

#include <memory>
#include <vector>
#include <cstdint>
#include <windows.h>
#include <winnt.h>

#include "../binary_structs.hpp"

namespace coff_generator {
    // returns a unique ptr to the final binary output to not overflow the stack
    std::unique_ptr<std::vector<uint8_t>> generate_coff_file(binary_structs::ObjectFile& obj_file);
}

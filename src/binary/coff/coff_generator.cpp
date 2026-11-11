#include <iostream>
#include <memory>
#include <vector>
#include <windows.h>
#include <winnt.h>

#include "../../../include/binary/binary_structs.hpp"
#include "../../../include/containers/arena_container.hpp"

namespace coff_generator::details {
    IMAGE_FILE_HEADER generate_file_header(const binary_structs::ObjectFile& obj_file) {
        IMAGE_FILE_HEADER file_header{};

        file_header.Machine =                   IMAGE_FILE_MACHINE_AMD64;
        file_header.NumberOfSections =          obj_file.sections.count;

        return file_header;
    }
}

namespace coff_generator {
    std::unique_ptr<std::vector<uint8_t>> generate_coff_file(binary_structs::ObjectFile& obj_file) {

    }
}

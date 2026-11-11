#include <iostream>
#include <memory>
#include <vector>
#include <windows.h>
#include <winnt.h>

#include "../../../include/binary/binary_structs.hpp"
#include "../../../include/containers/arena_container.hpp"

using namespace binary_structs;

namespace coff_generator::details {
    IMAGE_FILE_HEADER generate_file_header(const ObjectFile& obj_file) {
        IMAGE_FILE_HEADER file_header{};

        file_header.Machine =                   IMAGE_FILE_MACHINE_AMD64;
        file_header.NumberOfSections =          obj_file.sections.count;
        file_header.TimeDateStamp =             0;
        file_header.PointerToSymbolTable =      obj_file.pointer_to_symbols();
        file_header.NumberOfSymbols =           obj_file.symbol_table.count;

        return file_header;
    }

    std::vector<IMAGE_SECTION_HEADER> generate_section_headers(ObjectFile& obj_file) {
        std::vector<IMAGE_SECTION_HEADER> section_headers;

        uintptr_t pointer_to_current_section_data = obj_file.pointer_to_machinecode_start();
        uintptr_t pointer_to_current_section_relocation = obj_file.pointer_to_relocations_start();

        for (auto i{0}; i < obj_file.sections.count; ++i) {
            SectionStructure& section_structure = obj_file.sections[i].value();
            IMAGE_SECTION_HEADER section_header{};

            memcpy(
                section_header.Name,
                section_structure.name.data(),
                std::min(section_structure.name.size(), sizeof(section_header.Name))
            );

            section_header.SizeOfRawData =              section_structure.data.count;
            section_header.PointerToRawData =           pointer_to_current_section_data;
            section_header.PointerToRelocations =       pointer_to_current_section_relocation;
            section_header.NumberOfRelocations =        section_structure.relocations.count;
            section_header.Characteristics =            section_structure.characteristics;

            pointer_to_current_section_data +=              section_structure.data.count;
            pointer_to_current_section_relocation +=        section_structure.relocations.count * sizeof(IMAGE_RELOCATION);

            section_headers.push_back(section_header);
        }

        return section_headers;
    }
}

namespace coff_generator {
    std::unique_ptr<std::vector<uint8_t>> generate_coff_file(ObjectFile& obj_file) {
        memoryallocator::ArenaAllocator<uint8_t> raw_binary_bytes;
        IMAGE_FILE_HEADER file_header =                             details::generate_file_header(obj_file);
        std::vector<IMAGE_SECTION_HEADER> section_headers =         details::generate_section_headers(obj_file);
    }
}

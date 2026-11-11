#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
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
        file_header.NumberOfSections =          obj_file.sections->count;
        file_header.TimeDateStamp =             0;
        file_header.PointerToSymbolTable =      obj_file.pointer_to_symbols();
        file_header.NumberOfSymbols =           obj_file.symbol_table->count;

        return file_header;
    }

    std::vector<IMAGE_SECTION_HEADER> generate_section_headers(ObjectFile& obj_file) {
        std::vector<IMAGE_SECTION_HEADER> section_headers;

        uintptr_t pointer_to_current_section_data = obj_file.pointer_to_machinecode_start();
        uintptr_t pointer_to_current_section_relocation = obj_file.pointer_to_relocations_start();

        for (SectionStructure& section : *obj_file.sections) {
            IMAGE_SECTION_HEADER section_header{};

            memcpy(
                section_header.Name,
                section.name.data(),
                std::min(section.name.size(), sizeof(section_header.Name))
            );

            section_header.SizeOfRawData =                  section.data->count;
            section_header.PointerToRawData =               pointer_to_current_section_data;
            section_header.PointerToRelocations =           pointer_to_current_section_relocation;
            section_header.NumberOfRelocations =            section.relocations->count;
            section_header.Characteristics =                section.characteristics;

            // change the pointers to the next section
            pointer_to_current_section_data +=              section.data->count;
            if (section.image_relocation_active)
                pointer_to_current_section_relocation +=    section.relocations->count * sizeof(IMAGE_RELOCATION);

            section_headers.push_back(section_header);
        }

        return section_headers;
    }
}

namespace coff_generator {
    std::unique_ptr<containers::ArenaVector<uint8_t>> generate_coff_file(ObjectFile& obj_file) {
        auto raw_binary_bytes = std::make_unique<containers::ArenaVector<uint8_t>>();

        auto file_header =                             details::generate_file_header(obj_file);
        auto section_headers =         details::generate_section_headers(obj_file);

        uint8_t* file_header_bytes = reinterpret_cast<uint8_t*>(&file_header);
        uint8_t* section_headers_bytes = reinterpret_cast<uint8_t*>(section_headers.data());

        if (auto ok = raw_binary_bytes->insert_at_end(file_header_bytes, sizeof(file_header)); !ok) [[unlikely]]
            throw std::runtime_error("failed to allocate file header bytes to the raw binary output");

        if (auto ok = raw_binary_bytes->insert_at_end(section_headers_bytes, sizeof(section_headers)); !ok) [[unlikely]]
            throw std::runtime_error("failed to allocate section headers bytes to the raw binary output");

        for (auto& section_struct : *obj_file.sections) {
            if (auto ok = raw_binary_bytes->insert_at_end(section_struct.data->begin(), section_struct.data->count); !ok) [[unlikely]]
                throw std::runtime_error("failed to allocate machine code");
        }

        // writing the relocations right after the sections as raw bytes
        for (auto& section_struct : *obj_file.sections) {
            size_t relocations_size = section_struct.relocations->count * sizeof(IMAGE_RELOCATION);
            uint8_t* relocations = reinterpret_cast<uint8_t*>(section_struct.relocations->begin());

            if (auto ok = raw_binary_bytes->insert_at_end(relocations, relocations_size); !ok) [[unlikely]]
                throw std::runtime_error("failed to allocate relocations");
        }

        if (auto ok = raw_binary_bytes->insert_at_end(obj_file.string_table.data(), obj_file.string_table.size()); !ok) [[unlikely]]
            throw std::runtime_error("failed to allocate string table");

        return raw_binary_bytes;
    }
}

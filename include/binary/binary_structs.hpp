#pragma once

#include "../containers/arena_container.hpp"
#include <memory>
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
        std::shared_ptr<containers::ArenaVector<uint8_t>> data;
        std::shared_ptr<containers::ArenaVector<IMAGE_RELOCATION>> relocations;
        DWORD characteristics;
        bool image_relocation_active;

        SectionStructure() = default;

        SectionStructure(SectionStructure&& target_section) 
            : name(std::move(target_section.name)), data(std::move(target_section.data)), relocations(std::move(target_section.relocations)) {}
    };

    struct ObjectFile {
        std::shared_ptr<containers::ArenaVector<SectionStructure>> sections;
        std::shared_ptr<containers::ArenaVector<IMAGE_SYMBOL>> symbol_table;
        std::vector<unsigned char> string_table;

        ObjectFile(ObjectFile&& object_to_move)
            : sections(std::move(object_to_move.sections)),
            symbol_table(std::move(object_to_move.symbol_table)),
            string_table(std::move(object_to_move.string_table)) {}

        // the pointer to the symbol_table lies right after the relocations so calculate the distance to it
        const uintptr_t pointer_to_symbols() const {
            uintptr_t total = 0;

            total += sizeof(IMAGE_FILE_HEADER);
            total += get_sizes_of_section_headers();
            total += get_sizes_of_sections();
            total += get_sizes_of_relocations();

            return total;
        }

        const uintptr_t pointer_to_machinecode_start() const {
            return sizeof(IMAGE_FILE_HEADER) + get_sizes_of_section_headers();
        }

        const uintptr_t pointer_to_relocations_start() {
            return sizeof(IMAGE_FILE_HEADER) + get_sizes_of_section_headers() + get_sizes_of_sections();
        }

        // ----------------------------------------------------------------------------------------------------------

        const size_t get_sizes_of_section_headers() const {
            return sections.count * sizeof(IMAGE_SECTION_HEADER);
        }


        const size_t get_sizes_of_sections() const {
            size_t total = 0;

            for (const SectionStructure& section : sections) {
                total += section.data.count;
            }

            return total;
        }

        const size_t get_sizes_of_relocations() const {
            return sections.count * sizeof(IMAGE_RELOCATION);
        }
    };
}

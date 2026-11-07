#include <cstddef>
#include <cstdint>
#include <cstring>
#include <minwindef.h>
#include <span>
#include <unordered_map>
#include <vector>
#include <windows.h>
#include <winnt.h>

#include "../../include/binary/peformatter.hpp"
#include "../../include/binary/peformatter_headers.hpp"
#include "../../include/binary/peformatter_details.hpp"
#include "../../include/config/config.hpp"
#include "../../include/utility.hpp"

using namespace utility;
using namespace config;

namespace peformatter
{
    IMAGE_FILE_HEADER generate_file_header()
    {
        IMAGE_FILE_HEADER file_header{};

        file_header.Machine =               IMAGE_FILE_MACHINE_AMD64;
        file_header.NumberOfSections =      3;
        file_header.TimeDateStamp =         details::seconds_since_utc().count();
        file_header.PointerToSymbolTable =  0;                                  // not required
        file_header.NumberOfSymbols =       0;                                  // not required
        file_header.SizeOfOptionalHeader =  sizeof(IMAGE_OPTIONAL_HEADER64);
        file_header.Characteristics =       config::FILE_HEADER_CHARACTERISTICS;
        
        return file_header;
    }

    IMAGE_OPTIONAL_HEADER64 generate_optional_header(
        const std::unordered_map<details::PESectionNames, IMAGE_SECTION_HEADER>& section_headers,
        const DWORD entry_point_rva,
        const std::size_t size_of_sections_data,
        const std::size_t size_of_headers
    )
    {
        IMAGE_SECTION_HEADER text_section = section_headers.at(details::PESectionNames::Text);
        IMAGE_SECTION_HEADER data_section = section_headers.at(details::PESectionNames::Data);
        IMAGE_SECTION_HEADER rdata_section = section_headers.at(details::PESectionNames::RData);
        IMAGE_SECTION_HEADER bss_section = section_headers.at(details::PESectionNames::Bss);
        IMAGE_SECTION_HEADER idata_section = section_headers.at(details::PESectionNames::IData);

        IMAGE_OPTIONAL_HEADER64 optional_header{};

        optional_header.Magic =                     IMAGE_NT_OPTIONAL_HDR64_MAGIC;
        optional_header.MajorLinkerVersion =        0;                              // completely optional
        optional_header.MinorLinkerVersion =        0;                              // completely optional

        optional_header.SizeOfCode =                text_section.Misc.VirtualSize;
        optional_header.SizeOfInitializedData =     data_section.Misc.VirtualSize + rdata_section.Misc.VirtualSize + idata_section.Misc.VirtualSize;
        optional_header.SizeOfUninitializedData =   bss_section.Misc.VirtualSize;

        optional_header.AddressOfEntryPoint =       entry_point_rva;                // address of the first instruction that runs
        optional_header.BaseOfCode =                entry_point_rva;                // address from base address to the .code section
        optional_header.ImageBase =                 IMAGE_BASE_ADDRESS;
        optional_header.SectionAlignment =          SECTION_ALIGNMENT;              // how the sections are aligned
        optional_header.FileAlignment =             FILE_ALIGNMENT;                 // how the executable is aligned as a file

        optional_header.MajorOperatingSystemVersion = 6;                            // the operating system version required
        optional_header.MinorOperatingSystemVersion = 0;                            // v6.0
        optional_header.MajorImageVersion =         1;                              // version of the program (v1.0) linker doesnt care about this
        optional_header.MinorImageVersion =         0;
        optional_header.MajorSubsystemVersion =     6;                              // the subsystem version can get wonky. v6.0 like we did is fine
        optional_header.MinorSubsystemVersion =     0;
        optional_header.Win32VersionValue =         0;                              // Must be 0
 
        optional_header.SizeOfImage =               align_up(details::get_image_size(section_headers), SECTION_ALIGNMENT);
        optional_header.SizeOfHeaders =             align_up(details::get_header_sizes(4), FILE_ALIGNMENT);
        optional_header.CheckSum =                  0;                              // only needed for drivers
        optional_header.Subsystem =                 IMAGE_SUBSYSTEM_WINDOWS_CUI;
        optional_header.DllCharacteristics =        OPTIONAL_HEADER_DLL_CHARACTERISTICS; // dll characteristics for the sake of security
                                                                                         //
        optional_header.SizeOfStackReserve =        STACK_RESERVE;                  // reserving 1MB for stack
        optional_header.SizeOfStackCommit =         0x1000;
        optional_header.SizeOfHeapReserve =         HEAP_RESERVE;
        optional_header.SizeOfHeapCommit =          0x1000;

        optional_header.LoaderFlags =               0;                              // obsolete
        
        optional_header.NumberOfRvaAndSizes =       IMAGE_NUMBEROF_DIRECTORY_ENTRIES;

        optional_header.DataDirectory[IMAGE_IMPORT_DESCRIPTOR]
        
        return optional_header;
    }
    
    std::expected<std::vector<std::uint8_t>, std::error_code> generate_pe_binary(
        const std::span<std::uint8_t> text_section_code,
        const std::span<std::uint8_t> data_section_code,
        const std::span<std::uint8_t> rdata_section_code
    )
    {
        std::vector<std::uint8_t> final_output(
            text_section_code.size() +
            data_section_code.size() +
            rdata_section_code.size()
        );

        auto dos_header = details::generate_dos_headers();
        final_output.resize(final_output.size() + sizeof(dos_header));

        // write the dos_header to our final_output
        std::memcpy(
            final_output.data() + final_output.size(),
            &dos_header,
            sizeof(dos_header)
        );
    }
}

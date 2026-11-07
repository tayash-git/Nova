#pragma once

#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <windows.h>
#include <chrono>
#include <winnt.h>

#include "../config/config.hpp"
#include "../utility.hpp"
#include "peformatter_headers.hpp"

using namespace utility;
using namespace config;

namespace peformatter::details
{
    enum class PESectionNames {
        Text,
        Data,
        RData,
        Bss,
        IData
    };

    inline std::chrono::seconds seconds_since_utc()
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        );
    }

    inline IMAGE_DOS_HEADER generate_dos_headers()
    {
        IMAGE_DOS_HEADER dos_header{};
        dos_header.e_magic = IMAGE_DOS_SIGNATURE;
        // offset to NT headers which lie right after dos headers
        dos_header.e_lfanew = sizeof(dos_header);

        return dos_header;
    }

    // The NT headers is a wrapper for the file headers alongside the optional headers and magic number
    inline IMAGE_NT_HEADERS64 generate_nt_headers()
    {
        IMAGE_NT_HEADERS64 nt_headers{};
        nt_headers.Signature = IMAGE_NT_SIGNATURE;

        nt_headers.OptionalHeader = generate_optional_header();
        nt_headers.FileHeader = generate_file_header();

        return nt_headers;
    }

    consteval size_t get_header_sizes(int sections_amount)
    {
        auto dos_header_size = sizeof(IMAGE_DOS_HEADER);
        auto nt_headers_size = sizeof(IMAGE_NT_HEADERS64);
        auto sections_size = sizeof(IMAGE_SECTION_HEADER) * sections_amount;

        auto size = dos_header_size + nt_headers_size + sections_size;

        return size;
    }

    constexpr size_t get_image_size(const std::unordered_map<PESectionNames, IMAGE_SECTION_HEADER>& sections)
    {
        auto last_section = sections.find(PESectionNames::IData);

        if (last_section == sections.end())
            throw std::runtime_error("bss section not found in the sections vector");
        
        // bss section is the last place in the image so if we just take the virtual address + the size we get the size of the image
        auto size = last_section->second.Misc.VirtualSize + last_section->second.VirtualAddress;

        return size;
    }
}

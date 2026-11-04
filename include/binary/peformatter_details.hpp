#pragma once

#include <windows.h>
#include <chrono>

namespace peformatter::details
{
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

    // The NT headers is a wrapper for the file headers alongside the optional headers
    // as well as a magic number
    inline IMAGE_NT_HEADERS64 generate_nt_headers()
    {
        extern IMAGE_FILE_HEADER generate_file_header();            // functions in the cpp file
        extern IMAGE_OPTIONAL_HEADER64 generate_optional_header();  // functions in the cpp file

        IMAGE_NT_HEADERS64 nt_headers{};
        nt_headers.Signature = IMAGE_NT_SIGNATURE;

        nt_headers.OptionalHeader = generate_optional_header();
        nt_headers.FileHeader = generate_file_header();

        return nt_headers;
    }
}

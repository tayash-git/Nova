#pragma once

#include <windows.h>
#include <winnt.h>
#include "../types.hpp"

using namespace types;

namespace config
{
    constexpr QWORD IMAGE_BASE_ADDRESS =            0x140000000ULL;
    constexpr DWORD FILE_HEADER_CHARACTERISTICS =   IMAGE_FILE_EXECUTABLE_IMAGE |       // marks an executable
                                                    IMAGE_FILE_LARGE_ADDRESS_AWARE |    // allows for addresses larger than 2 GB
                                                    IMAGE_FILE_RELOCS_STRIPPED;         // important for ASLR
    constexpr DWORD OPTIONAL_HEADER_DLL_CHARACTERISTICS = IMAGE_DLLCHARACTERISTICS_NX_COMPAT |
                                                          IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE;
    constexpr DWORD SECTION_ALIGNMENT =             0x10000;
    constexpr DWORD FILE_ALIGNMENT =                0x200;
    constexpr DWORD STACK_RESERVE =                 0x100000;
    constexpr DWORD HEAP_RESERVE =                  0x100000;
}

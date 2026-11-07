#pragma once

#include <windows.h>
#include <winnt.h>

namespace peformatter::details
{
    IMAGE_FILE_HEADER generate_file_header();
    IMAGE_OPTIONAL_HEADER64 generate_optional_header();
}

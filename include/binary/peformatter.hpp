#pragma once

/*
 * A namespace with functions that generates
 * the PE file format with the inputted sections
 * as raw machine code
 *
 * Structure of the PE File format:
 *
 * IMAGE_DOS_HEADER: old dos bullshit
 * IMAGE_NT_HEADERS: wrapper for the file header and optional header
 * IMAGE_SECTION_TABLE: array of all the sections
*/

#include <cstdint>
#include <expected>
#include <span>
#include <system_error>
#include <windows.h>
#include <winnt.h>
#include <chrono>

using namespace std::chrono_literals;

namespace peformatter
{
    std::expected<std::vector<std::uint8_t>, std::error_code> generate_pe_binary(
        const std::span<std::uint8_t> text_section_code,
        const std::span<std::uint8_t> data_section_code,
        const std::span<std::uint8_t> rdata_section_code
    );
}

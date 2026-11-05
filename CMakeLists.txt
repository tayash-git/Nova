cmake_minimum_required(VERSION 3.20)
project(NovaCompiler LANGUAGES CXX)

if(NOT WIN32)
    add_compile_options(--target=x86_64-w64-mingw32 -fms-extensions)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(novax ${SOURCES})

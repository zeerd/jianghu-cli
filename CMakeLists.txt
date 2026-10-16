cmake_minimum_required(VERSION 3.16)
project(jianghu-cli
    VERSION 1.0
    DESCRIPTION "金剑江湖 CLI Client"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # 统一使用静态 CRT，与 NuGet 的 GTest 包（rt-static）保持一致，
    # 避免运行时库不一致导致的 LNK2038。
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ---- Coverage build ----
option(COVERAGE "Enable LLVM source-based code coverage" OFF)
if(COVERAGE)
    # Require Clang for coverage
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "COVERAGE=ON requires Clang. "
                            "Use -DCMAKE_CXX_COMPILER=clang++")
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-instr-generate")
    message(STATUS "Coverage build enabled (LLVM source-based)")
endif()

# ---- Unit tests ----
option(BUILD_TESTS "Build unit tests" ON)

find_package(Threads REQUIRED)

if(WIN32)
    # 源码为 UTF-8（含中文字符串与注释），MSVC/clang-cl 必须显式声明。
    add_compile_options(
        "$<$<COMPILE_LANG_AND_ID:CXX,MSVC,Clang>:/utf-8>")
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
else()
    # iconv 仅在非 Windows 平台使用（Windows 走 Win32 API）
    find_package(Iconv REQUIRED)
    add_compile_definitions(LIBICONV_PLUG)
endif()

# Sources
file(GLOB_RECURSE SOURCES "src/*.cc")
add_library(jianghu_lib STATIC ${SOURCES})
target_include_directories(jianghu_lib PUBLIC src)
target_link_libraries(jianghu_lib PUBLIC Threads::Threads)

add_executable(jianghu src/main.cc)
target_include_directories(jianghu PRIVATE src)
target_link_libraries(jianghu PRIVATE jianghu_lib)

if(WIN32)
    # Winsock
    target_link_libraries(jianghu_lib PUBLIC ws2_32)
else()
    target_link_libraries(jianghu_lib PUBLIC Iconv::Iconv)
endif()

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

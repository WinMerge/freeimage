cmake_minimum_required(VERSION 3.24)
project(FreeImageARM64XTest LANGUAGES C CXX)

if(NOT WIN32 OR NOT MSVC OR NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(FATAL_ERROR "This experiment requires the Microsoft Windows toolchain")
endif()
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.41)
    message(FATAL_ERROR "ARM64X link-input capture requires VS 2022 17.11 or newer")
endif()

set(ARM64X_STAGE "Native" CACHE STRING "Native, Compatible, Hybrid, or Consumer")
set_property(CACHE ARM64X_STAGE PROPERTY STRINGS Native Compatible Hybrid Consumer)
set(ARM64X_NATIVE_RSP "" CACHE FILEPATH "Native ARM64 linker response file")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Both halves must use the same CRT. Keep the test artifacts self-contained.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
get_filename_component(FREEIMAGE_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

if(ARM64X_STAGE STREQUAL "Native" OR ARM64X_STAGE STREQUAL "Compatible" OR ARM64X_STAGE STREQUAL "Hybrid")
    if(ARM64X_STAGE STREQUAL "Native" AND NOT CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64")
        message(FATAL_ERROR "The Native stage requires -A ARM64")
    elseif(NOT ARM64X_STAGE STREQUAL "Native" AND NOT CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64EC")
        message(FATAL_ERROR "The Hybrid stage requires -A ARM64EC")
    endif()
    set(FREEIMAGE_STATIC OFF CACHE BOOL "" FORCE)
    set(FREEIMAGE_MSVC_RUNTIME MT CACHE STRING "" FORCE)
    set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(BUILD_JXR ON CACHE BOOL "" FORCE)
    set(BUILD_OPENEXR ON CACHE BOOL "" FORCE)
    set(BUILD_LIBHEIF OFF CACHE BOOL "" FORCE)
    set(BUILD_LIBJXL OFF CACHE BOOL "" FORCE)
    if(ARM64X_STAGE STREQUAL "Hybrid")
        # Windows-MSVC.cmake initializes this flag to /machine:ARM64EC.
        # MSBuild emits it after the target's /MACHINE:ARM64X option, so the
        # last switch silently turns the merged link back into plain EC.
        # Override only shared-library linking in this wrapper directory;
        # the compiler and the control/consumer builds remain ARM64EC.
        string(REGEX REPLACE
            "[-/][mM][aA][cC][hH][iI][nN][eE]:[aA][rR][mM]64[eE][cC]"
            "/MACHINE:ARM64X" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
    endif()
    add_subdirectory("${FREEIMAGE_SOURCE}" freeimage)
    if(ARM64X_STAGE STREQUAL "Native")
        target_link_options(FreeImage PRIVATE
            "/LINKREPROFULLPATHRSP:${CMAKE_BINARY_DIR}/native.rsp")
    elseif(ARM64X_STAGE STREQUAL "Hybrid")
        include("${CMAKE_CURRENT_LIST_DIR}/ReadNativeInputs.cmake")
        arm64x_read_native_inputs("${ARM64X_NATIVE_RSP}" native_objects native_libraries native_def)
        target_sources(FreeImage PRIVATE ${native_objects})
        target_link_libraries(FreeImage PRIVATE ${native_libraries})
        target_link_options(FreeImage PRIVATE /MACHINE:ARM64X)
        if(native_def)
            target_link_options(FreeImage PRIVATE "/DEFARM64NATIVE:${native_def}")
        endif()
        message(STATUS "ARM64X: merging native inputs from ${ARM64X_NATIVE_RSP}")
    endif()
elseif(NOT ARM64X_STAGE STREQUAL "Consumer")
    message(FATAL_ERROR "Invalid ARM64X_STAGE=${ARM64X_STAGE}")
endif()

# No import library: the executable must load the exact DLL passed on the CLI.
add_executable(ARM64XSmoke smoke.cpp)
target_include_directories(ARM64XSmoke PRIVATE "${FREEIMAGE_SOURCE}/Source")
target_compile_definitions(ARM64XSmoke PRIVATE _WIN32_WINNT=0x0A00)

# Select nested-build parallelism when the build runs, so the standard
# CMAKE_BUILD_PARALLEL_LEVEL environment variable can change without a
# reconfigure. Pass an explicit count because bare --parallel uses the
# native build tool's default, which is not always the logical CPU count.
if(NOT "${FREEIMAGE_CODEC_BUILD_JOBS}" STREQUAL "")
    set(jobs "${FREEIMAGE_CODEC_BUILD_JOBS}")
    set(source "FREEIMAGE_CODEC_BUILD_JOBS")
elseif(NOT "$ENV{CMAKE_BUILD_PARALLEL_LEVEL}" STREQUAL "")
    set(jobs "$ENV{CMAKE_BUILD_PARALLEL_LEVEL}")
    set(source "CMAKE_BUILD_PARALLEL_LEVEL")
else()
    include(ProcessorCount)
    ProcessorCount(jobs)
    if(jobs EQUAL 0)
        set(jobs 1)
    endif()
    set(source "detected logical CPUs")
endif()
if(NOT jobs MATCHES "^[1-9][0-9]*$")
    message(FATAL_ERROR "${source} must be a positive integer")
endif()
message(STATUS "Bundled codec build: ${jobs} parallel jobs (${source})")
if(FREEIMAGE_CODEC_ENGLISH_MESSAGES)
    # This script runs in its own process, so these overrides only reach
    # the nested build and its compiler/linker children (#236).
    set(ENV{VSLANG} 1033)
    set(ENV{DOTNET_CLI_UI_LANGUAGE} en-US)
    message(STATUS "Bundled codec diagnostics: English (FREEIMAGE_CODEC_ENGLISH_MESSAGES=ON)")
endif()
execute_process(COMMAND "${CMAKE_COMMAND}" --build "${FREEIMAGE_CODEC_BINARY_DIR}"
    --config "${FREEIMAGE_CODEC_CONFIG}" --parallel "${jobs}"
    RESULT_VARIABLE result)
if(NOT "${result}" STREQUAL "0")
    message(FATAL_ERROR "Bundled codec build failed: ${result}")
endif()

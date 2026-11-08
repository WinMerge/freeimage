# Bundled modern codecs. Source archives live in the repository: no network
# access, package manager, or pkg-config is required for this build path.
include_guard(GLOBAL)
if(CMAKE_VERSION VERSION_LESS 3.20)
    message(FATAL_ERROR "Bundled HEIF/JPEG XL builds require CMake 3.20 or newer; use USE_SYSTEM_LIBHEIF/USE_SYSTEM_LIBJXL with an older CMake")
endif()
include(ExternalProject)
include(GNUInstallDirs)

option(FREEIMAGE_CODEC_ENGLISH_MESSAGES "Use English diagnostics for nested MSVC codec builds" ON)
if(MSVC)
    if(FREEIMAGE_CODEC_ENGLISH_MESSAGES)
        message(STATUS "Bundled MSVC codec diagnostics: English (FREEIMAGE_CODEC_ENGLISH_MESSAGES=ON)")
    else()
        message(STATUS "Bundled MSVC codec diagnostics: inherited language (FREEIMAGE_CODEC_ENGLISH_MESSAGES=OFF)")
    endif()
endif()

# Empty selects automatic parallelism at build time: honor the standard
# CMAKE_BUILD_PARALLEL_LEVEL environment variable, then detect host CPUs.
set(FREEIMAGE_CODEC_BUILD_JOBS "" CACHE STRING "Parallel jobs per bundled codec (empty uses environment or CPU count)")
if(NOT "${FREEIMAGE_CODEC_BUILD_JOBS}" STREQUAL "" AND
   NOT FREEIMAGE_CODEC_BUILD_JOBS MATCHES "^[1-9][0-9]*$")
    message(FATAL_ERROR "FREEIMAGE_CODEC_BUILD_JOBS must be empty or a positive integer")
endif()

function(freeimage_bundled_project name archive hash)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs DEPENDS ARGS)
    cmake_parse_arguments(CODEC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    set(common
        -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTING=OFF
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_DEBUG_POSTFIX=
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
        -DCMAKE_INSTALL_PREFIX:PATH=${_codec_prefix}
        -DCMAKE_INSTALL_LIBDIR=lib
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
        -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
        -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
        -DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_EXE_LINKER_FLAGS}
        -DCMAKE_DISABLE_FIND_PACKAGE_PkgConfig=TRUE)
    # Forward target configuration, including the selected MSVC CRT, not
    # host-specific dependency paths. Each upstream project chooses its own
    # language standard in its isolated build.
    foreach(var CMAKE_TOOLCHAIN_FILE CMAKE_SYSROOT CMAKE_OSX_SYSROOT
                CMAKE_OSX_DEPLOYMENT_TARGET CMAKE_OSX_ARCHITECTURES
                CMAKE_SYSTEM_NAME CMAKE_SYSTEM_PROCESSOR
                CMAKE_ANDROID_NDK CMAKE_ANDROID_ARCH_ABI CMAKE_SYSTEM_VERSION
                ANDROID_ABI ANDROID_PLATFORM ANDROID_STL
                CMAKE_MSVC_RUNTIME_LIBRARY CMAKE_C_COMPILER_TARGET CMAKE_CXX_COMPILER_TARGET)
        if(DEFINED ${var} AND NOT "${${var}}" STREQUAL "")
            string(REPLACE ";" "|" value "${${var}}")
            list(APPEND common "-D${var}:STRING=${value}")
        endif()
    endforeach()
    # Highway's CMake minimum is older than 3.15, so CMP0091 would otherwise
    # remain unset/OLD and it would ignore CMAKE_MSVC_RUNTIME_LIBRARY.
    if(DEFINED CMAKE_MSVC_RUNTIME_LIBRARY AND MSVC)
        list(APPEND common "-DCMAKE_POLICY_DEFAULT_CMP0091=NEW")
    endif()
    if(POLICY CMP0135)
        cmake_policy(SET CMP0135 NEW)
    endif()
    set(patch_args)
    if(name STREQUAL "jxl")
        set(patch_args PATCH_COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=<SOURCE_DIR>
            -P "${PROJECT_SOURCE_DIR}/cmake/patches/libjxl-no-pkg-config.cmake")
    endif()
    set(configure_args)
    set(english_messages OFF)
    if(MSVC AND FREEIMAGE_CODEC_ENGLISH_MESSAGES)
        set(english_messages ON)
        # MSBuild 17.14 can emit localized UTF-8 text into a pipe whose
        # reader expects a legacy code page (#236). Use English only in
        # these child processes; do not change the developer's console.
        # Configure with the same language as the build so Ninja detects
        # the correct /showIncludes prefix during compiler identification.
        # Use a separate compiler cache when adopting this language setting.
        # Existing Ninja trees may have cached a localized /showIncludes prefix.
        set(configure_args
            BINARY_DIR "${PROJECT_BINARY_DIR}/codec-build/${name}/build-msvc-en"
            CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env
            VSLANG=1033 DOTNET_CLI_UI_LANGUAGE=en-US
            ${CMAKE_COMMAND} -S <SOURCE_DIR> -B <BINARY_DIR>
            -G "${CMAKE_GENERATOR}" ${common} ${CODEC_ARGS})
        if(CMAKE_GENERATOR_PLATFORM)
            list(APPEND configure_args -A "${CMAKE_GENERATOR_PLATFORM}")
        endif()
        if(CMAKE_GENERATOR_TOOLSET)
            list(APPEND configure_args -T "${CMAKE_GENERATOR_TOOLSET}")
        endif()
        if(CMAKE_GENERATOR_INSTANCE)
            list(APPEND configure_args "-DCMAKE_GENERATOR_INSTANCE:INTERNAL=${CMAKE_GENERATOR_INSTANCE}")
        endif()
    endif()
    ExternalProject_Add(freeimage_${name}
        URL "${PROJECT_SOURCE_DIR}/Source/ThirdParty/${archive}"
        URL_HASH "SHA256=${hash}"
        PREFIX "${PROJECT_BINARY_DIR}/codec-build/${name}"
        INSTALL_DIR "${PROJECT_BINARY_DIR}/codec-install"
        ${patch_args}
        ${configure_args}
        CMAKE_ARGS ${common} ${CODEC_ARGS}
        LIST_SEPARATOR |
        BUILD_COMMAND ${CMAKE_COMMAND}
            "-DFREEIMAGE_CODEC_BINARY_DIR:PATH=<BINARY_DIR>"
            "-DFREEIMAGE_CODEC_CONFIG:STRING=$<CONFIG>"
            "-DFREEIMAGE_CODEC_BUILD_JOBS:STRING=${FREEIMAGE_CODEC_BUILD_JOBS}"
            "-DFREEIMAGE_CODEC_ENGLISH_MESSAGES:BOOL=${english_messages}"
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/BuildBundledCodec.cmake"
        INSTALL_COMMAND ${CMAKE_COMMAND} --install <BINARY_DIR> --config $<CONFIG>
        DEPENDS ${CODEC_DEPENDS}
        EXCLUDE_FROM_ALL TRUE)
    ExternalProject_Add_StepDependencies(freeimage_${name} build
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/BuildBundledCodec.cmake")
endfunction()

function(freeimage_build_bundled_codec codec)
    set(_codec_prefix "${PROJECT_BINARY_DIR}/codec-install/$<CONFIG>")
    # Imported include directories must exist at generation time.
    file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/codec-install/include")
    foreach(config ${CMAKE_CONFIGURATION_TYPES} "${CMAKE_BUILD_TYPE}")
        file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/codec-install/${config}/include")
    endforeach()
    set(lib "${_codec_prefix}/lib")
    set(pre "${CMAKE_STATIC_LIBRARY_PREFIX}")
    set(ext "${CMAKE_STATIC_LIBRARY_SUFFIX}")
    find_package(Threads REQUIRED)
    if(codec STREQUAL "HEIF")
        set(de265 "${pre}de265${ext}")
        if(WIN32)
            set(de265 "libde265${ext}")
        endif()
        set(kvazaar "${pre}kvazaar${ext}")
        if(MSVC)
            set(kvazaar "libkvazaar${ext}")
        endif()
        freeimage_bundled_project(de265 libde265-1.1.3.tar.gz
            554228bd17788c99a7e63b37ab5634722190e6e2bf60c1dcb01cef328e133905
            ARGS -DENABLE_DECODER=OFF -DENABLE_ENCODER=OFF -DENABLE_SDL=OFF
                 -DENABLE_SIMD=OFF -DENABLE_AVX2=OFF -DENABLE_AVX512=OFF)
        # kvazaar is an HEVC encoder only; read-only builds leave it out.
        set(kvazaar_args -DWITH_KVAZAAR=OFF)
        set(kvazaar_names)
        set(kvazaar_depends)
        if(FREEIMAGE_HEIF_EXPORT)
            freeimage_bundled_project(kvazaar kvazaar-2.3.2.tar.gz
                ddd0038696631ca5368d8e40efee36d2bbb805854b9b1dda8b12ea9b397ea951
                ARGS -DBUILD_TESTS=OFF -DBUILD_KVAZAAR_BINARY=OFF -DUSE_CRYPTO=OFF -DGIT_SUBMODULE=OFF)
            set(kvazaar_args -DWITH_KVAZAAR=ON
                -DKVAZAAR_INCLUDE_DIR:PATH=${_codec_prefix}/include
                -DKVAZAAR_LIBRARY:FILEPATH=${lib}/${kvazaar})
            set(kvazaar_names "${kvazaar}")
            set(kvazaar_depends freeimage_kvazaar)
        endif()
        set(aom_args -DWITH_AOM_DECODER=OFF -DWITH_AOM_ENCODER=OFF)
        set(aom_names)
        set(aom_depends)
        if(FREEIMAGE_BUNDLE_AOM)
            if(FREEIMAGE_HEIF_EXPORT)
                set(aom_encoder ON)
                set(aom_config_encoder 1)
            else()
                set(aom_encoder OFF)
                set(aom_config_encoder 0)
            endif()
            # libaom's RTCD generator requires Perl even for the generic CPU
            # target. Keep AV1 enabled by default, but allow HEIF-only builds
            # on toolchains where Perl is unavailable.
            freeimage_bundled_project(aom libaom-3.15.3.tar.gz
                12c01dc731ed7b5a999e28120b45bf58a002ba3435a4ba2cfbc1e084a27c9c04
                ARGS -DAOM_TARGET_CPU=generic -DENABLE_NASM=OFF -DCONFIG_RUNTIME_CPU_DETECT=0
                     -DCONFIG_AV1_ENCODER=${aom_config_encoder}
                     -DENABLE_APPS=OFF -DENABLE_DOCS=OFF -DENABLE_EXAMPLES=OFF
                     -DENABLE_TESTDATA=OFF -DENABLE_TESTS=OFF -DENABLE_TOOLS=OFF)
            set(aom_args -DWITH_AOM_DECODER=ON -DWITH_AOM_ENCODER=${aom_encoder}
                -DAOM_INCLUDE_DIR:PATH=${_codec_prefix}/include
                -DAOM_LIBRARY:FILEPATH=${lib}/${pre}aom${ext}
                -DCMAKE_DISABLE_FIND_PACKAGE_AOM=FALSE -DAOM_DIR:PATH=${lib}/cmake/AOM
                -Daom_usage_flag_exists:INTERNAL=TRUE)
            set(aom_names "${pre}aom${ext}")
            set(aom_depends freeimage_aom)
        endif()
        freeimage_bundled_project(heif libheif-1.23.7.tar.gz
            f7483e818503ea6224c8863e4a56ac02c7089021a98472c811c12aa19faf2475
            DEPENDS ${aom_depends} freeimage_de265 ${kvazaar_depends}
            ARGS -DCMAKE_CXX_STANDARD=20 -DBUILD_DOCUMENTATION=OFF -DENABLE_PLUGIN_LOADING=OFF -DWITH_EXAMPLES=OFF
                 -DWITH_GDK_PIXBUF=OFF -DWITH_FUZZERS=OFF -DENABLE_EXPERIMENTAL_FEATURES=OFF
                 -DWITH_HEADER_COMPRESSION=OFF -DWITH_UNCOMPRESSED_CODEC=OFF
                 -DWITH_X265=OFF -DWITH_X264=OFF -DWITH_OpenH264_DECODER=OFF
                 -DWITH_DAV1D=OFF -DWITH_SvtEnc=OFF -DWITH_RAV1E=OFF
                 -DWITH_VVDEC=OFF -DWITH_VVENC=OFF -DWITH_UVG266=OFF
                 -DWITH_JPEG_DECODER=OFF -DWITH_JPEG_ENCODER=OFF
                 -DWITH_OpenJPEG_DECODER=OFF -DWITH_OpenJPEG_ENCODER=OFF
                 -DWITH_OPENJPH_ENCODER=OFF -DWITH_FFMPEG_DECODER=OFF
                 -DWITH_LIBSHARPYUV=OFF
                 -DWITH_LIBDE265=ON ${kvazaar_args} ${aom_args}
                 -DLIBDE265_INCLUDE_DIR:PATH=${_codec_prefix}/include
                 -DLIBDE265_LIBRARY:FILEPATH=${lib}/${de265}
                 "-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS} -DLIBDE265_STATIC_BUILD=1 -DKVZ_STATIC_LIB=1")
        set(names "${pre}heif${ext}" "${de265}" ${kvazaar_names} ${aom_names})
        set(project heif)
        set(defines LIBHEIF_STATIC_BUILD)
    elseif(codec STREQUAL "JXL")
        freeimage_bundled_project(highway highway-1.4.0.tar.gz
            e72241ac9524bb653ae52ced768b508045d4438726a303f10181a38f764a453c
            ARGS -DHWY_ENABLE_TESTS=OFF -DHWY_ENABLE_EXAMPLES=OFF -DHWY_ENABLE_CONTRIB=OFF)
        freeimage_bundled_project(brotli brotli-1.2.1.tar.gz
            98d35d11a78dc453aebb7d10c3748f3f73ff78c8267c0dc697b8631f67bf7e8f
            ARGS -DBROTLI_DISABLE_TESTS=ON -DBROTLI_BUILD_TOOLS=OFF)
        freeimage_bundled_project(lcms2 lcms2-2.20.zip
            5f8a1385cc05157575e249ae893f893889c3f6ea77e553245d6867a1fe658918
            ARGS -DLCMS2_BUILD_SHARED=OFF -DLCMS2_BUILD_STATIC=ON -DLCMS2_BUILD_TESTS=OFF
                 -DLCMS2_BUILD_TOOLS=OFF -DLCMS2_WITH_JPEG=OFF -DLCMS2_WITH_TIFF=OFF -DLCMS2_WITH_ZLIB=OFF)
        set(lcms "${pre}lcms2${ext}")
        if(MSVC)
            set(lcms "${pre}lcms2$<$<CONFIG:Debug>:d>${ext}")
        endif()
        freeimage_bundled_project(jxl libjxl-0.12.0.tar.gz
            03e9be69a30be4011f559da75328b6d7cea8ad921fabfbd551ce10bf45cdc992
            DEPENDS freeimage_highway freeimage_brotli freeimage_lcms2
            ARGS -DCMAKE_CXX_STANDARD=17 -DCMAKE_PREFIX_PATH:PATH=${_codec_prefix}
                 -DJPEGXL_ENABLE_TOOLS=OFF -DJPEGXL_ENABLE_DEVTOOLS=OFF
                 -DJPEGXL_ENABLE_FUZZERS=OFF -DPROVISION_DEPENDENCIES=OFF
                 -DJPEGXL_ENABLE_BENCHMARK=OFF -DJPEGXL_ENABLE_EXAMPLES=OFF -DJPEGXL_ENABLE_JNI=OFF
                 -DJPEGXL_ENABLE_MANPAGES=OFF -DJPEGXL_ENABLE_DOXYGEN=OFF
                 -DJPEGXL_ENABLE_OPENEXR=OFF -DJPEGXL_ENABLE_SJPEG=OFF
                 -DJPEGXL_ENABLE_SKCMS=OFF -DJPEGXL_ENABLE_PLUGINS=OFF
                 -DJPEGXL_ENABLE_VIEWERS=OFF -DJPEGXL_ENABLE_TCMALLOC=OFF
                 -DJPEGXL_WARNINGS_AS_ERRORS=OFF -DJPEGXL_BUNDLE_LIBPNG=OFF
                 -DJPEGXL_FORCE_SYSTEM_HWY=ON -DJPEGXL_FORCE_SYSTEM_BROTLI=ON -DJPEGXL_FORCE_SYSTEM_LCMS2=ON
                 -DHWY_INCLUDE_DIR:PATH=${_codec_prefix}/include
                 -DHWY_LIBRARY:FILEPATH=${lib}/${pre}hwy${ext}
                 -DBROTLI_INCLUDE_DIR:PATH=${_codec_prefix}/include
                 -DBROTLICOMMON_LIBRARY:FILEPATH=${lib}/${pre}brotlicommon${ext}
                 -DBROTLIENC_LIBRARY:FILEPATH=${lib}/${pre}brotlienc${ext}
                 -DBROTLIDEC_LIBRARY:FILEPATH=${lib}/${pre}brotlidec${ext}
                 -DLCMS2_INCLUDE_DIR:PATH=${_codec_prefix}/include
                 -DLCMS2_LIBRARY:FILEPATH=${lib}/${lcms})
        set(names "${pre}jxl${ext}" "${pre}jxl_cms${ext}" "${pre}hwy${ext}"
                  "${pre}brotlienc${ext}" "${pre}brotlidec${ext}" "${pre}brotlicommon${ext}" "${lcms}")
        set(project jxl)
        set(defines JXL_STATIC_DEFINE JXL_CMS_STATIC_DEFINE)
    endif()
    add_library(FreeImageCodec::${codec} INTERFACE IMPORTED GLOBAL)
    set(paths)
    set(installed_names)
    foreach(name IN LISTS names)
        list(APPEND paths "${lib}/${name}")
        # Keep the installed private archive names independent of build type.
        string(REPLACE "$<$<CONFIG:Debug>:d>" "" installed_name "${name}")
        list(APPEND installed_names "${installed_name}")
        install(FILES "${lib}/${name}" DESTINATION ${CMAKE_INSTALL_LIBDIR}/freeimage-codecs
            RENAME "${installed_name}" COMPONENT Development)
    endforeach()
    set(extra_libraries)
    if(ANDROID)
        list(APPEND extra_libraries log)
    endif()
    set_target_properties(FreeImageCodec::${codec} PROPERTIES
        INTERFACE_LINK_LIBRARIES "${paths};Threads::Threads;${CMAKE_DL_LIBS};${extra_libraries}"
        INTERFACE_INCLUDE_DIRECTORIES "${_codec_prefix}/include"
        INTERFACE_COMPILE_DEFINITIONS "${defines}")
    # ExternalProject has no library targets visible to Ninja; record its
    # installed archives as byproducts so downstream links have build rules.
    ExternalProject_Add_Step(freeimage_${project} archive_byproducts
        COMMAND ${CMAKE_COMMAND} -E true
        DEPENDEES install BYPRODUCTS ${paths})
    add_dependencies(FreeImageCodec::${codec} freeimage_${project})
    set(FreeImage_BUNDLED_${codec}_ARCHIVES "${installed_names}" PARENT_SCOPE)
endfunction()

# Bundled modern codecs. Source archives live in the repository: no network
# access, package manager, or pkg-config is required for this build path.
include_guard(GLOBAL)
if(CMAKE_VERSION VERSION_LESS 3.20)
    message(FATAL_ERROR "Bundled HEIF/JPEG XL builds require CMake 3.20 or newer; use USE_SYSTEM_LIBHEIF/USE_SYSTEM_LIBJXL with an older CMake")
endif()
include(ExternalProject)
include(GNUInstallDirs)

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
    # Forward target configuration, not host-specific dependency paths. Each
    # upstream project chooses its own language standard in its isolated build.
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
    if(POLICY CMP0135)
        cmake_policy(SET CMP0135 NEW)
    endif()
    ExternalProject_Add(freeimage_${name}
        URL "${PROJECT_SOURCE_DIR}/Source/ThirdParty/${archive}"
        URL_HASH "SHA256=${hash}"
        PREFIX "${PROJECT_BINARY_DIR}/codec-build/${name}"
        INSTALL_DIR "${_codec_prefix}"
        CMAKE_ARGS ${common} ${CODEC_ARGS}
        LIST_SEPARATOR |
        BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config $<CONFIG> --parallel 2
        INSTALL_COMMAND ${CMAKE_COMMAND} --install <BINARY_DIR> --config $<CONFIG>
        DEPENDS ${CODEC_DEPENDS}
        EXCLUDE_FROM_ALL TRUE)
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
        # The portable AOM backend avoids requiring NASM and is usable on
        # x86, ARM, Android API 25 and cross toolchains without host detection.
        freeimage_bundled_project(aom libaom-3.15.1.tar.gz
            8ca0c52746174603500f0adb6f2a215d69c9ca2aab2acb3caa06fb791d8d01bf
            ARGS -DAOM_TARGET_CPU=generic -DENABLE_NASM=OFF -DCONFIG_RUNTIME_CPU_DETECT=0
                 -DENABLE_APPS=OFF -DENABLE_DOCS=OFF -DENABLE_EXAMPLES=OFF
                 -DENABLE_TESTDATA=OFF -DENABLE_TESTS=OFF -DENABLE_TOOLS=OFF)
        freeimage_bundled_project(de265 libde265-1.1.3.tar.gz
            554228bd17788c99a7e63b37ab5634722190e6e2bf60c1dcb01cef328e133905
            ARGS -DENABLE_DECODER=OFF -DENABLE_ENCODER=OFF -DENABLE_SDL=OFF
                 -DENABLE_SIMD=OFF -DENABLE_AVX2=OFF -DENABLE_AVX512=OFF)
        freeimage_bundled_project(kvazaar kvazaar-2.3.2.tar.gz
            ddd0038696631ca5368d8e40efee36d2bbb805854b9b1dda8b12ea9b397ea951
            ARGS -DBUILD_TESTS=OFF -DBUILD_KVAZAAR_BINARY=OFF -DUSE_CRYPTO=OFF -DGIT_SUBMODULE=OFF)
        freeimage_bundled_project(heif libheif-1.23.5.tar.gz
            fd9036064c4432f0550d15072ddf34956a248279ee9aeaff0fba3fa0f77d8f1a
            DEPENDS freeimage_aom freeimage_de265 freeimage_kvazaar
            ARGS -DCMAKE_CXX_STANDARD=20 -DENABLE_PLUGIN_LOADING=OFF -DWITH_EXAMPLES=OFF
                 -DWITH_GDK_PIXBUF=OFF -DWITH_FUZZERS=OFF -DENABLE_EXPERIMENTAL_FEATURES=OFF
                 -DWITH_HEADER_COMPRESSION=OFF -DWITH_UNCOMPRESSED_CODEC=OFF
                 -DWITH_X265=OFF -DWITH_X264=OFF -DWITH_OpenH264_DECODER=OFF
                 -DWITH_DAV1D=OFF -DWITH_SvtEnc=OFF -DWITH_RAV1E=OFF
                 -DWITH_VVDEC=OFF -DWITH_VVENC=OFF -DWITH_UVG266=OFF
                 -DWITH_JPEG_DECODER=OFF -DWITH_JPEG_ENCODER=OFF
                 -DWITH_OpenJPEG_DECODER=OFF -DWITH_OpenJPEG_ENCODER=OFF
                 -DWITH_OPENJPH_ENCODER=OFF -DWITH_FFMPEG_DECODER=OFF
                 -DWITH_LIBSHARPYUV=OFF
                 -DWITH_LIBDE265=ON -DWITH_KVAZAAR=ON -DWITH_AOM_DECODER=ON -DWITH_AOM_ENCODER=ON
                 -DLIBDE265_INCLUDE_DIR:PATH=${_codec_prefix}/include
                 -DLIBDE265_LIBRARY:FILEPATH=${lib}/${de265}
                 -DKVAZAAR_INCLUDE_DIR:PATH=${_codec_prefix}/include
                 -DKVAZAAR_LIBRARY:FILEPATH=${lib}/${kvazaar}
                 -DAOM_INCLUDE_DIR:PATH=${_codec_prefix}/include
                 -DAOM_LIBRARY:FILEPATH=${lib}/${pre}aom${ext}
                 -DCMAKE_DISABLE_FIND_PACKAGE_AOM=FALSE -DAOM_DIR:PATH=${lib}/cmake/AOM
                 -Daom_usage_flag_exists:INTERNAL=TRUE
                 "-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS} -DLIBDE265_STATIC_BUILD=1 -DKVZ_STATIC_LIB=1")
        set(names "${pre}heif${ext}" "${de265}" "${kvazaar}" "${pre}aom${ext}")
        set(project heif)
        set(defines LIBHEIF_STATIC_BUILD)
    elseif(codec STREQUAL "JXL")
        freeimage_bundled_project(highway highway-1.4.0.tar.gz
            e72241ac9524bb653ae52ced768b508045d4438726a303f10181a38f764a453c
            ARGS -DHWY_ENABLE_TESTS=OFF -DHWY_ENABLE_EXAMPLES=OFF -DHWY_ENABLE_CONTRIB=OFF)
        freeimage_bundled_project(brotli brotli-1.2.0.tar.gz
            816c96e8e8f193b40151dad7e8ff37b1221d019dbcb9c35cd3fadbfe6477dfec
            ARGS -DBROTLI_DISABLE_TESTS=ON -DBROTLI_BUILD_TOOLS=OFF)
        freeimage_bundled_project(lcms2 lcms2-2.19.1.zip
            06d59795cb87a67f73daaa5b6f86bf77b256710aad5822e3fee7f89546a5f93f
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
                 -DJPEGXL_ENABLE_BENCHMARK=OFF -DJPEGXL_ENABLE_EXAMPLES=OFF -DJPEGXL_ENABLE_JNI=OFF
                 -DJPEGXL_ENABLE_JPEGLI=OFF -DJPEGXL_ENABLE_JPEGLI_LIBJPEG=OFF
                 -DJPEGXL_ENABLE_MANPAGES=OFF -DJPEGXL_ENABLE_DOXYGEN=OFF
                 -DJPEGXL_ENABLE_OPENEXR=OFF -DJPEGXL_ENABLE_SJPEG=OFF
                 -DJPEGXL_ENABLE_SKCMS=OFF -DJPEGXL_ENABLE_PLUGINS=OFF
                 -DJPEGXL_ENABLE_VIEWERS=OFF -DJPEGXL_ENABLE_TCMALLOC=OFF
                 -DJPEGXL_WARNINGS_AS_ERRORS=OFF -DJPEGXL_BUNDLE_LIBPNG=OFF
                 -DJPEGXL_FORCE_SYSTEM_HWY=ON -DJPEGXL_FORCE_SYSTEM_BROTLI=ON -DJPEGXL_FORCE_SYSTEM_LCMS2=ON
                 -DHWY_DIR:PATH=${lib}/cmake/hwy
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

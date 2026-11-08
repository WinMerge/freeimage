
add_executable(MainTestSuite 
    MainTestSuite.cpp
    testChannels.cpp
    testHeaderOnly.cpp
    testImageType.cpp
    testJPEG.cpp
    testMemIO.cpp
    testMPage.cpp
    testMPageMemory.cpp
    testMPageStream.cpp
    testPlugins.cpp
    testThumbnail.cpp
    testTools.cpp
    testWrappedBuffer.cpp
)

if (BUILD_TESTS)
    target_link_libraries(MainTestSuite FreeImage)
    target_compile_definitions(MainTestSuite PRIVATE FREEIMAGE_TEST_LIBJXL=$<BOOL:${BUILD_LIBJXL}>)
    target_compile_definitions(MainTestSuite PRIVATE FREEIMAGE_TEST_LIBHEIF=$<BOOL:${BUILD_LIBHEIF}>)
    if (DEFINED PNG_LIBRARY)
        target_link_libraries(MainTestSuite ${PNG_LIBRARY})
    endif()
    if (DEFINED ZLIB_LIBRARY)
        target_link_libraries(MainTestSuite ${ZLIB_LIBRARY})
    endif()

    if(APPLE)
    set_target_properties(MainTestSuite PROPERTIES
        MACOSX_BUNDLE TRUE
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.yourcompany.MainTestSuite"
    )
    endif()

    # Windows searches beside the executable for linked DLLs. The library
    # lives in the parent build directory, while tests run from TestAPI.
    if(WIN32 AND NOT FREEIMAGE_STATIC)
        add_custom_command(TARGET MainTestSuite POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:FreeImage> $<TARGET_FILE_DIR:MainTestSuite>
            VERBATIM)
    endif()

    add_test(NAME MainTestSuite WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/TestAPI COMMAND $<TARGET_FILE:MainTestSuite>)

    add_executable(LargeFileTIFFTest testLargeFileTIFF.cpp)
    target_link_libraries(LargeFileTIFFTest PRIVATE FreeImage)
    if(UNIX AND NOT APPLE)
        # The fixture writer needs the same large-file ABI as the library.
        target_compile_definitions(LargeFileTIFFTest PRIVATE _FILE_OFFSET_BITS=64)
    endif()
    if(WIN32 AND NOT FREEIMAGE_STATIC)
        add_custom_command(TARGET LargeFileTIFFTest POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:FreeImage> $<TARGET_FILE_DIR:LargeFileTIFFTest> VERBATIM)
    endif()
    add_test(NAME LargeFileTIFFTest COMMAND LargeFileTIFFTest)
    set_tests_properties(LargeFileTIFFTest PROPERTIES TIMEOUT 30)

    # The Python release-fixture runner supplies one downloaded input at a time.
    add_executable(IO64FixtureDecoder testIO64FixtureDecoder.cpp)
    target_link_libraries(IO64FixtureDecoder PRIVATE FreeImage)
    file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/io64-decoder-$<CONFIG>.txt"
        CONTENT "$<TARGET_FILE:IO64FixtureDecoder>\n")
    if(UNIX AND NOT APPLE)
        target_compile_definitions(IO64FixtureDecoder PRIVATE _FILE_OFFSET_BITS=64)
    endif()
    if(MINGW)
        target_link_options(IO64FixtureDecoder PRIVATE -municode)
    endif()
    if(WIN32 AND NOT FREEIMAGE_STATIC)
        add_custom_command(TARGET IO64FixtureDecoder POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:FreeImage> $<TARGET_FILE_DIR:IO64FixtureDecoder> VERBATIM)
    endif()

    add_executable(IffPbmRleTest testIffPbmRle.cpp)
    target_link_libraries(IffPbmRleTest PRIVATE FreeImage)
    if(WIN32 AND NOT FREEIMAGE_STATIC)
        add_custom_command(TARGET IffPbmRleTest POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:FreeImage> $<TARGET_FILE_DIR:IffPbmRleTest>
            VERBATIM)
    endif()
    add_test(NAME IffPbmRleTest COMMAND IffPbmRleTest)
    set_tests_properties(IffPbmRleTest PROPERTIES TIMEOUT 5)

    add_executable(GifPlaybackBoundsTest testGifPlaybackBounds.cpp)
    target_link_libraries(GifPlaybackBoundsTest PRIVATE FreeImage)
    if(WIN32 AND NOT FREEIMAGE_STATIC)
        add_custom_command(TARGET GifPlaybackBoundsTest POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:FreeImage> $<TARGET_FILE_DIR:GifPlaybackBoundsTest>
            VERBATIM)
    endif()
    add_test(NAME GifPlaybackBoundsTest COMMAND GifPlaybackBoundsTest)
    set_tests_properties(GifPlaybackBoundsTest PROPERTIES TIMEOUT 10)

    add_executable(RasWordLoopTest testRasWordLoop.cpp)
    target_link_libraries(RasWordLoopTest PRIVATE FreeImage)
    if(WIN32 AND NOT FREEIMAGE_STATIC)
        add_custom_command(TARGET RasWordLoopTest POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:FreeImage> $<TARGET_FILE_DIR:RasWordLoopTest>
            VERBATIM)
    endif()
    add_test(NAME RasWordLoopTest COMMAND RasWordLoopTest)
    set_tests_properties(RasWordLoopTest PROPERTIES TIMEOUT 10)

    add_executable(MemWritePositionTest testMemWritePosition.cpp)
    target_link_libraries(MemWritePositionTest PRIVATE FreeImage)
    if(WIN32 AND NOT FREEIMAGE_STATIC)
        add_custom_command(TARGET MemWritePositionTest POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:FreeImage> $<TARGET_FILE_DIR:MemWritePositionTest>
            VERBATIM)
    endif()
    add_test(NAME MemWritePositionTest COMMAND MemWritePositionTest)
    set_tests_properties(MemWritePositionTest PROPERTIES TIMEOUT 10)

    add_executable(BmpRleCursorTest testBmpRleCursor.cpp)
    target_link_libraries(BmpRleCursorTest PRIVATE FreeImage)
    if(WIN32 AND NOT FREEIMAGE_STATIC)
        add_custom_command(TARGET BmpRleCursorTest POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:FreeImage> $<TARGET_FILE_DIR:BmpRleCursorTest>
            VERBATIM)
    endif()
    add_test(NAME BmpRleCursorTest COMMAND BmpRleCursorTest)
    set_tests_properties(BmpRleCursorTest PROPERTIES TIMEOUT 60)
endif ()

if(BUILD_TESTS AND BUILD_LIBOPENJPEG)
    add_executable(J2KCallbacksTest testJ2KCallbacks.cpp)
    target_link_libraries(J2KCallbacksTest PRIVATE FreeImage)
    if(WIN32 AND NOT FREEIMAGE_STATIC)
        add_custom_command(TARGET J2KCallbacksTest POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:FreeImage> $<TARGET_FILE_DIR:J2KCallbacksTest>
            VERBATIM)
    endif()
    add_test(NAME J2KCallbacksTest COMMAND J2KCallbacksTest)
    set_tests_properties(J2KCallbacksTest PROPERTIES TIMEOUT 30)
endif()

# These tests call vendored codec APIs directly to create inputs that exercise
# FreeImage's loaders. Those internal symbols are not part of its Windows DLL API.
if(BUILD_TESTS AND FREEIMAGE_STATIC AND BUILD_OPENEXR AND NOT USE_SYSTEM_OPENEXR)
    add_executable(OpenJphNltTest testOpenJphNlt.cpp)
    target_link_libraries(OpenJphNltTest PRIVATE FreeImage)
    add_test(NAME OpenJphNltTest COMMAND OpenJphNltTest)
    set_tests_properties(OpenJphNltTest PROPERTIES TIMEOUT 10)
endif()

if(BUILD_TESTS AND FREEIMAGE_STATIC AND BUILD_OPENEXR AND NOT USE_SYSTEM_OPENEXR
   AND BUILD_LIBTIFF AND NOT USE_SYSTEM_LIBTIFF)
    add_executable(VendorCodecTest testVendorCodecs.cpp)
    target_link_libraries(VendorCodecTest PRIVATE FreeImage)
    target_compile_definitions(VendorCodecTest PRIVATE
        FREEIMAGE_TEST_OPENJPH_SIMD=$<BOOL:${FREEIMAGE_OPENJPH_SIMD_ENABLED}>)
    add_test(NAME VendorCodecTest COMMAND VendorCodecTest)
endif()


if(BUILD_TESTS AND BUILD_LIBHEIF)
    add_executable(HeifCodecTest testHeif.cpp)
    target_link_libraries(HeifCodecTest PRIVATE FreeImage)
    target_compile_definitions(HeifCodecTest PRIVATE
        FREEIMAGE_TEST_HEIF_EXPORT=$<BOOL:${FREEIMAGE_HEIF_EXPORT}>)
    if(NOT USE_SYSTEM_LIBHEIF)
        target_compile_definitions(HeifCodecTest PRIVATE FREEIMAGE_TEST_BUNDLED_HEIF)
        if(FREEIMAGE_BUNDLE_AOM)
            target_compile_definitions(HeifCodecTest PRIVATE FREEIMAGE_TEST_BUNDLED_AOM)
        endif()
    endif()
    add_executable(GenerateHeifFixtures EXCLUDE_FROM_ALL heif/generate.cpp)
    target_link_libraries(GenerateHeifFixtures PRIVATE FreeImageCodec::HEIF)
    if(WIN32 AND NOT FREEIMAGE_STATIC)
        add_custom_command(TARGET HeifCodecTest POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:FreeImage> $<TARGET_FILE_DIR:HeifCodecTest> VERBATIM)
    endif()
    add_test(NAME HeifCodecTest COMMAND HeifCodecTest)
    set_tests_properties(HeifCodecTest PROPERTIES TIMEOUT 120
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/heif")
endif()

if(BUILD_TESTS)
    add_executable(SaveErrorTest testSaveErrors.cpp)
    target_link_libraries(SaveErrorTest PRIVATE FreeImage)
    if(WIN32 AND NOT FREEIMAGE_STATIC)
        add_custom_command(TARGET SaveErrorTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:FreeImage> $<TARGET_FILE_DIR:SaveErrorTest>)
    endif()
    add_test(NAME SaveErrorTest COMMAND SaveErrorTest)
endif()

if(BUILD_TESTS)
    add_executable(DoubleRGBFTest testDoubleRGBF.cpp)
    target_link_libraries(DoubleRGBFTest PRIVATE FreeImage)
    if(WIN32 AND NOT FREEIMAGE_STATIC)
        add_custom_command(TARGET DoubleRGBFTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:FreeImage> $<TARGET_FILE_DIR:DoubleRGBFTest>)
    endif()
    add_test(NAME DoubleRGBFTest COMMAND DoubleRGBFTest)
endif()

if(BUILD_TESTS AND BUILD_LIBHEIF)
    add_test(NAME HeifEncodingTest COMMAND HeifCodecTest --encode)
    set_tests_properties(HeifEncodingTest PROPERTIES TIMEOUT 120 WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/heif")
    if(USE_SYSTEM_LIBHEIF AND UNIX AND NOT APPLE)
        # Ubuntu's external x265 encoder leaks its configuration object. Keep
        # decoder tests unsuppressed, and suppress only that encoder allocator.
        set_tests_properties(HeifEncodingTest PROPERTIES ENVIRONMENT
            "LSAN_OPTIONS=suppressions=${CMAKE_CURRENT_SOURCE_DIR}/heif/x265-lsan.supp")
    endif()
endif()

if(BUILD_TESTS AND BUILD_LIBJXL)
    add_executable(JpegXLTest testJpegXL.cpp)
    target_link_libraries(JpegXLTest PRIVATE FreeImage)
    target_compile_definitions(JpegXLTest PRIVATE
        FREEIMAGE_TEST_JXL_EXPORT=$<BOOL:${FREEIMAGE_JXL_EXPORT}>)
    if(WIN32 AND NOT FREEIMAGE_STATIC)
        add_custom_command(TARGET JpegXLTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:FreeImage> $<TARGET_FILE_DIR:JpegXLTest>)
    endif()
    add_test(NAME JpegXLTest COMMAND JpegXLTest "${CMAKE_CURRENT_SOURCE_DIR}/jxl")
    set_tests_properties(JpegXLTest PROPERTIES TIMEOUT 60)
endif()

if(BUILD_TESTS)
    add_executable(DependencyInfoTest testDependencies.cpp)
    target_link_libraries(DependencyInfoTest PRIVATE FreeImage)
    if(WIN32 AND NOT FREEIMAGE_STATIC)
        add_custom_command(TARGET DependencyInfoTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:FreeImage> $<TARGET_FILE_DIR:DependencyInfoTest>)
    endif()
    add_test(NAME DependencyInfoTest COMMAND DependencyInfoTest)
endif()

if(BUILD_TESTS)
    add_executable(ImageAnalysisTest testAnalysis.cpp)
    target_link_libraries(ImageAnalysisTest PRIVATE FreeImage)
    if(WIN32 AND NOT FREEIMAGE_STATIC)
        add_custom_command(TARGET ImageAnalysisTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:FreeImage> $<TARGET_FILE_DIR:ImageAnalysisTest>)
    endif()
    add_test(NAME ImageAnalysisTest COMMAND ImageAnalysisTest)
endif()

if(BUILD_TESTS AND BUILD_LIBTIFF)
    add_executable(UnicodeMultiPageTest testUnicodeMultiPage.cpp)
    target_link_libraries(UnicodeMultiPageTest PRIVATE FreeImage)
    if(WIN32 AND NOT FREEIMAGE_STATIC)
        add_custom_command(TARGET UnicodeMultiPageTest POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:FreeImage> $<TARGET_FILE_DIR:UnicodeMultiPageTest>)
    endif()
    add_test(NAME UnicodeMultiPageTest COMMAND UnicodeMultiPageTest)
endif()

if(BUILD_TESTS)
    add_executable(CodecDisplayTest testCodecDisplay.cpp)
    target_link_libraries(CodecDisplayTest PRIVATE FreeImage)
    target_compile_definitions(CodecDisplayTest PRIVATE
        FREEIMAGE_TEST_JP2=$<BOOL:${BUILD_LIBOPENJPEG}>
        FREEIMAGE_TEST_JXR=$<BOOL:${BUILD_JXR}>)
    add_test(NAME CodecDisplayTest COMMAND CodecDisplayTest
        "${CMAKE_CURRENT_BINARY_DIR}/codec-display-$<CONFIG>.jp2")
    set_tests_properties(CodecDisplayTest PROPERTIES TIMEOUT 60)

    add_executable(CodecDiagnostics EXCLUDE_FROM_ALL ../Examples/codec-diagnostics/main.cpp)
    target_link_libraries(CodecDiagnostics PRIVATE FreeImage)
    target_compile_features(CodecDiagnostics PRIVATE cxx_std_11)
    if(WIN32 AND NOT FREEIMAGE_STATIC)
        foreach(target CodecDisplayTest CodecDiagnostics)
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:FreeImage> $<TARGET_FILE_DIR:${target}> VERBATIM)
        endforeach()
    endif()
endif()

if(BUILD_TESTS AND FREEIMAGE_METADATA_STRIP)
    find_package(Threads REQUIRED)
    add_executable(MetadataStripTest testMetadataStrip.cpp)
    target_link_libraries(MetadataStripTest PRIVATE FreeImage Threads::Threads)
    if(WIN32 AND NOT FREEIMAGE_STATIC)
        add_custom_command(TARGET MetadataStripTest POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:FreeImage> $<TARGET_FILE_DIR:MetadataStripTest> VERBATIM)
    endif()
    add_test(NAME MetadataStripTest COMMAND MetadataStripTest)
    set_tests_properties(MetadataStripTest PROPERTIES TIMEOUT 60)
endif()

# Each executable needs its own DLL directory. Parallel vcpkg app-local steps
# scan/copy dependencies while the commands above deploy FreeImage.dll; sharing
# a directory lets one target overwrite the DLL another target is reading.
if(WIN32 AND BUILD_TESTS)
    get_property(test_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
    foreach(test_target IN LISTS test_targets)
        get_target_property(test_type ${test_target} TYPE)
        if(test_type STREQUAL "EXECUTABLE")
            set_target_properties(${test_target} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${test_target}")
        endif()
    endforeach()
endif()

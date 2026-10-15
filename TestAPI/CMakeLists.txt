
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
endif ()

# These tests call vendored codec APIs directly to create inputs that exercise
# FreeImage's loaders. Those internal symbols are not part of its Windows DLL API.
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
    add_executable(GenerateHeifFixtures EXCLUDE_FROM_ALL heif/generate.cpp)
    target_link_libraries(GenerateHeifFixtures PRIVATE PkgConfig::LIBHEIF)
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

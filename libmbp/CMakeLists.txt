include(ExternalProject)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Allow libmbp headers to be found
include_directories(${CMAKE_SOURCE_DIR})

include_directories(${MBP_ZLIB_INCLUDES})
include_directories(${MBP_LIBLZMA_INCLUDES})
include_directories(${MBP_LIBARCHIVE_INCLUDES})

include_directories(${CMAKE_SOURCE_DIR}/external)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/version.h
    @ONLY
)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

set(MBP_SOURCES
    bootimage.cpp
    cpiofile.cpp
    device.cpp
    fileinfo.cpp
    patcherconfig.cpp
    private/fileutils.cpp
    private/logging.cpp
    private/stringutils.cpp
    bootimage/androidformat.cpp
    bootimage/bumpformat.cpp
    bootimage/bumppatcher.cpp
    bootimage/fileformat.cpp
    bootimage/lokiformat.cpp
    bootimage/lokipatcher.cpp
    bootimage/sonyelfformat.cpp
    edify/tokenizer.cpp
    # C wrapper API
    cwrapper/cbootimage.cpp
    cwrapper/ccommon.cpp
    cwrapper/ccpiofile.cpp
    cwrapper/cdevice.cpp
    cwrapper/cfileinfo.cpp
    cwrapper/cpatcherconfig.cpp
    cwrapper/cpatcherinterface.cpp
    cwrapper/private/util.cpp
    # Autopatchers
    autopatchers/standardpatcher.cpp
    autopatchers/xposedpatcher.cpp
    # Patchers
    patchers/mbtoolupdater.cpp
    patchers/multibootpatcher.cpp
    # Ramdisk patchers
    ramdiskpatchers/core.cpp
    ramdiskpatchers/default.cpp
    ramdiskpatchers/pepper.cpp
    external/sha.cpp
)

# If we're building for Android, then compile with ndk-build since it can easily
# handle building for multiple architectures

if(MBP_BUILD_ANDROID)
    # Wordaround semicolon separated list issue
    set(MBP_SOURCES_STR "")
    foreach(_source ${MBP_SOURCES})
        set(MBP_SOURCES_STR "${MBP_SOURCES_STR} ${_source}")
    endforeach()

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Android.mk.in
        ${CMAKE_CURRENT_BINARY_DIR}/Android.mk
        @ONLY
    )

    ExternalProject_Add(
        mbp
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/out/
        BUILD_COMMAND ${NDK_BUILD}
                      -C <SOURCE_DIR>
                      -j${PROCESSOR_COUNT}
                      NDK_PROJECT_PATH=.
                      NDK_APPLICATION_MK=Application.mk
                      NDK_APP_LIBS_OUT=${CMAKE_CURRENT_BINARY_DIR}/out
                      NDK_OUT=<TMP_DIR>
                      APP_BUILD_SCRIPT=${CMAKE_CURRENT_BINARY_DIR}/Android.mk
                      ${NDK_ARGS}
        INSTALL_COMMAND ""
    )

    # Always rebuild since files may have changed
    ExternalProject_Add_Step(
        mbp
        forcerebuild
        DEPENDEES update
        DEPENDERS configure
        ALWAYS 1
        COMMAND ""
    )

    add_dependencies(
        mbp
        mbpio-android
    )
else()
    # For desktop machines, we'll build with cmake

    add_definitions(
        -DLIBMBP_LIBRARY
        -DSTRICTZIPUNZIP
    )

    add_library(mbp SHARED ${MBP_SOURCES})

    if(NOT MSVC)
        set_target_properties(mbp PROPERTIES
            CXX_STANDARD 11
            CXX_STANDARD_REQUIRED 1
            C_STANDARD 99
            C_STANDARD_REQUIRED 1
        )
    endif()

    target_link_libraries(mbp
        mbpio
        ${MBP_ZLIB_LIBRARIES}
        ${MBP_LIBLZMA_LIBRARIES}
        ${MBP_LIBARCHIVE_LIBRARIES}
        minizip
    )

    if(UNIX)
        target_link_libraries(mbp pthread)
    endif()

    # Install library
    install(
        TARGETS mbp
        LIBRARY DESTINATION ${LIB_INSTALL_DIR} COMPONENT Libraries
        RUNTIME DESTINATION ${LIB_INSTALL_DIR} COMPONENT Libraries
        ARCHIVE DESTINATION ${LIB_INSTALL_DIR} COMPONENT Libraries
    )
endif()


################################################################################
# libmbp-mini for Android
################################################################################

set(MBP_MINI_SOURCES
    bootimage.cpp
    cpiofile.cpp
    device.cpp
    patcherconfig.cpp
    private/fileutils.cpp
    private/logging.cpp
    private/stringutils.cpp
    cwrapper/cbootimage.cpp
    cwrapper/ccommon.cpp
    cwrapper/ccpiofile.cpp
    cwrapper/cdevice.cpp
    cwrapper/cpatcherconfig.cpp
    cwrapper/private/util.cpp
    bootimage/androidformat.cpp
    bootimage/bumpformat.cpp
    bootimage/bumppatcher.cpp
    bootimage/fileformat.cpp
    bootimage/lokiformat.cpp
    bootimage/lokipatcher.cpp
    bootimage/sonyelfformat.cpp
    external/sha.cpp
)

# Wordaround semicolon separated list issue
set(MBP_SOURCES_STR "")
foreach(_source ${MBP_MINI_SOURCES})
    set(MBP_SOURCES_STR "${MBP_SOURCES_STR} ${_source}")
endforeach()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Android.mk.in
    ${CMAKE_CURRENT_BINARY_DIR}/Android-mini.mk
    @ONLY
)

ExternalProject_Add(
    mbp-mini
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/out-mini/
    BUILD_COMMAND ${NDK_BUILD}
                  -C <SOURCE_DIR>
                  -j${PROCESSOR_COUNT}
                  NDK_PROJECT_PATH=.
                  NDK_APPLICATION_MK=Application.mk
                  NDK_APP_LIBS_OUT=${CMAKE_CURRENT_BINARY_DIR}/out-mini
                  NDK_OUT=<TMP_DIR>
                  APP_BUILD_SCRIPT=${CMAKE_CURRENT_BINARY_DIR}/Android-mini.mk
                  ${NDK_ARGS}
                  MBP_MINI=true
    INSTALL_COMMAND ""
)

# Always rebuild since files may have changed
ExternalProject_Add_Step(
    mbp-mini
    forcerebuild
    DEPENDEES update
    DEPENDERS configure
    ALWAYS 1
    COMMAND ""
)

add_dependencies(
    mbp-mini
    mbpio-android
)

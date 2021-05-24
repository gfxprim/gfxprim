cmake_minimum_required(VERSION 3.11)
project(
    GFXprim
    DESCRIPTION "GFXprim is Open-source modular 2D bitmap graphics library with emphasis on speed and correctness."
    VERSION 1.0.0
    LANGUAGES C
)

# Options
option(GFXPRIM_BUILD_STATIC "Build Static" ON)
option(GFXPRIM_BUILD_SHARED "Build Shared" OFF)
option(GFXPRIM_DISABLE_PNG "Disable PNG support" OFF)
option(GFXPRIM_DISABLE_SDL "Disable SDL support" OFF)
option(GFXPRIM_DISABLE_JPEG "Disable JPEG support" OFF)
option(GFXPRIM_DISABLE_WEBP "Disable WebP support" OFF)
option(GFXPRIM_DISABLE_OPENJPEG "Disable OpenJPEG support" OFF)
option(GFXPRIM_DISABLE_GIFLIB "Disable gif support" OFF)
option(GFXPRIM_DISABLE_TIFF "Disable TIFF support" OFF)
option(GFXPRIM_DISABLE_ZLIB "Disable zlib support" OFF)
option(GFXPRIM_DISABLE_LIBARCHIVE "Disable libarchive support" OFF)
option(GFXPRIM_DISABLE_LIBXCB "Disable libxcb support" OFF)
option(GFXPRIM_DISABLE_X_SHM "Disable MIT-SHM X Extension support" OFF)
option(GFXPRIM_DISABLE_AALIB "Disable AALIB support" OFF)
option(GFXPRIM_DISABLE_FREETYPE "Disable Freetype" OFF)
option(GFXPRIM_DISABLE_JSON "Disable JSON support" OFF)
option(GFXPRIM_DISABLE_DL "Disable Dynamic Linker" OFF)
option(GFXPRIM_DISABLE_V4L2 "Disable video for Linux 2" OFF)
option(GFXPRIM_DISABLE_PTHREAD "Disable Posix Threads" OFF)
option(GFXPRIM_DISABLE_BACKTRACE "Disable C stack trace writeout" OFF)
option(GFXPRIM_BUILD_TEMPLATES "Build Templates" ON)

# Variables
set(GFXPRIM_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)
set(GFXPRIM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(GFXPRIM_INCLUDE_DIRS ${GFXPRIM_INCLUDE_DIR})
set(GFXPRIM_DEFINES)
set(GFXPRIM_COMPILE_OPTIONS)
set(GFXPRIM_LIBRARIES)

# Templates
if(GFXPRIM_BUILD_TEMPLATES)
    find_package(Python QUIET)
    if(Python_FOUND)
        message(STATUS "GFXprim: [x] python " ${Python_VERSION})

        set(GFXPRIM_CCT ${CMAKE_CURRENT_SOURCE_DIR}/gen/bin/cct.py)
        set(GFXPRIM_CCT_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/gen/include)

        file(GLOB GFXPRIM_T_FILES
            ${GFXPRIM_SOURCE_DIR}/backends/*.gen.c.t
            ${GFXPRIM_SOURCE_DIR}/core/*.gen.c.t
            ${GFXPRIM_SOURCE_DIR}/filters/*.gen.c.t
            ${GFXPRIM_SOURCE_DIR}/gfx/*.gen.c.t
            ${GFXPRIM_SOURCE_DIR}/grabbers/*.gen.c.t
            ${GFXPRIM_SOURCE_DIR}/input/*.gen.c.t
            ${GFXPRIM_SOURCE_DIR}/loaders/*.gen.c.t
            ${GFXPRIM_SOURCE_DIR}/text/*.gen.c.t
            ${GFXPRIM_SOURCE_DIR}/utils/*.gen.c.t
            ${GFXPRIM_SOURCE_DIR}/widgets/*.gen.c.t
            ${GFXPRIM_INCLUDE_DIR}/backends/*.gen.h.t
            ${GFXPRIM_INCLUDE_DIR}/core/*.gen.h.t
            ${GFXPRIM_INCLUDE_DIR}/filters/*.gen.h.t
            ${GFXPRIM_INCLUDE_DIR}/gfx/*.gen.h.t
            ${GFXPRIM_INCLUDE_DIR}/grabbers/*.gen.h.t
            ${GFXPRIM_INCLUDE_DIR}/input/*.gen.h.t
            ${GFXPRIM_INCLUDE_DIR}/loaders/*.gen.h.t
            ${GFXPRIM_INCLUDE_DIR}/text/*.gen.h.t
            ${GFXPRIM_INCLUDE_DIR}/utils/*.gen.h.t
            ${GFXPRIM_INCLUDE_DIR}/widgets/*.gen.h.t
        )

        foreach(GFXPRIM_T_FILE ${GFXPRIM_T_FILES})
            string(REPLACE ".t" "" OUTPUT_FILE ${GFXPRIM_T_FILE})
            if(NOT EXISTS ${OUTPUT_FILE})
                # TODO: Use a better solution around setting PYTHONPATH.
                execute_process(COMMAND PYTHONPATH="${GFXPRIM_CCT_INCLUDE}" ${Python_EXECUTABLE} "${GFXPRIM_CCT}" -I "${GFXPRIM_CCT_INCLUDE}" -o "${OUTPUT_FILE}" "${GFXPRIM_T_FILE}"
                    WORKING_DIRECTORY ${GFXPRIM_CCT_INCLUDE}
                    ERROR_VARIABLE GFXPRIM_CCT_ERR
                )
                if(GFXPRIM_CCT_ERR)
                    message(STATUS "GFXprim: [ ] template ${OUTPUT_FILE}")
                else()
                    message(STATUS "GFXprim: [x] template ${OUTPUT_FILE}")
                endif()
            endif()
        endforeach()
    else()
        message(STATUS "GFXprim: [ ] python")
    endif()
endif()

# Sources
file(GLOB GFXPRIM_SOURCES
    ${GFXPRIM_SOURCE_DIR}/backends/*.c
    ${GFXPRIM_SOURCE_DIR}/core/*.c
    ${GFXPRIM_SOURCE_DIR}/filters/*.c
    ${GFXPRIM_SOURCE_DIR}/gfx/*.c
    ${GFXPRIM_SOURCE_DIR}/grabbers/*.c
    ${GFXPRIM_SOURCE_DIR}/input/*.c
    ${GFXPRIM_SOURCE_DIR}/loaders/*.c
    ${GFXPRIM_SOURCE_DIR}/text/*.c
    ${GFXPRIM_SOURCE_DIR}/utils/*.c
)

# Dependencies
if(NOT GFXPRIM_DISABLE_PNG)
    find_package(PNG QUIET)
    if(PNG_FOUND)
        list(APPEND GFXPRIM_DEFINES HAVE_LIBPNG)
        list(APPEND GFXPRIM_LIBRARIES ${PNG_LIBRARY})
        list(APPEND GFXPRIM_INCLUDE_DIRS ${PNG_INCLUDE_DIR})
        message(STATUS "GFXprim: [x] png " ${PNG_VERSION_STRING})
    else()
        message(STATUS "GFXprim: [ ] png")
    endif()
endif()

if(NOT GFXPRIM_DISABLE_SDL)
    find_package(SDL QUIET)
    if(SDL_FOUND)
        list(APPEND GFXPRIM_DEFINES HAVE_LIBSDL)
        list(APPEND GFXPRIM_LIBRARIES ${SDL_LIBRARIES})
        list(APPEND GFXPRIM_INCLUDE_DIRS ${SDL_INCLUDE_DIRS})
        message(STATUS "GFXprim: [x] sdl " ${SDL_VERSION})
    else()
        message(STATUS "GFXprim: [ ] sdl")
    endif()
endif()

if(NOT GFXPRIM_DISABLE_JPEG)
    find_package(JPEG QUIET)
    if(JPEG_FOUND)
        list(APPEND GFXPRIM_DEFINES HAVE_JPEG)
        list(APPEND GFXPRIM_LIBRARIES ${JPEG_LIBRARIES})
        list(APPEND GFXPRIM_INCLUDE_DIRS ${JPEG_INCLUDE_DIRS})
        message(STATUS "GFXprim: [x] jpeg " ${JPEG_VERSION})
    else()
        message(STATUS "GFXprim: [ ] jpeg")
    endif()
endif()

if(NOT GFXPRIM_DISABLE_WEBP)
    find_package(WebP QUIET)
    if(WebP_FOUND)
        list(APPEND GFXPRIM_DEFINES HAVE_WEBP)
        list(APPEND GFXPRIM_LIBRARIES ${WebP_LIBRARIES})
        list(APPEND GFXPRIM_INCLUDE_DIRS ${WebP_INCLUDE_DIRS})
        list(APPEND GFXPRIM_COMPILE_OPTIONS ${WebP_COMPILE_OPTIONS})
        message(STATUS "GFXprim: [x] webp " ${WebP_VERSION})
    else()
        message(STATUS "GFXprim: [ ] webp")
    endif()
endif()

if(NOT GFXPRIM_DISABLE_OPENJPEG)
    find_package(OpenJPEG QUIET)
    if(OpenJPEG_FOUND)
        list(APPEND GFXPRIM_DEFINES HAVE_OPENJPEG)
        list(APPEND GFXPRIM_LIBRARIES ${OPENJPEG_LIBRARIES})
        list(APPEND GFXPRIM_INCLUDE_DIRS ${OPENJPEG_INCLUDE_DIRS})
        message(STATUS "GFXprim: [x] openjpeg " ${OpenJPEG_VERSION})
    else()
        message(STATUS "GFXprim: [ ] openjpeg")
    endif()
endif()

if(NOT GFXPRIM_DISABLE_GIFLIB)
    find_package(GIF QUIET)
    if(GIF_FOUND)
        list(APPEND GFXPRIM_DEFINES HAVE_GIFLIB)
        list(APPEND GFXPRIM_LIBRARIES ${GIF_LIBRARIES})
        list(APPEND GFXPRIM_INCLUDE_DIRS ${GIF_INCLUDE_DIR})
        message(STATUS "GFXprim: [x] giflib " ${GIF_VERSION})
    else()
        message(STATUS "GFXprim: [ ] giflib")
    endif()
endif()

if(NOT GFXPRIM_DISABLE_TIFF)
    find_package(TIFF QUIET)
    if(TIFF_FOUND)
        list(APPEND GFXPRIM_DEFINES HAVE_TIFF)
        list(APPEND GFXPRIM_LIBRARIES ${TIFF_LIBRARIES})
        list(APPEND GFXPRIM_INCLUDE_DIRS ${TIFF_INCLUDE_DIRS})
        message(STATUS "GFXprim: [x] tiff")
    else()
        message(STATUS "GFXprim: [ ] tiff")
    endif()
endif()

if(NOT GFXPRIM_DISABLE_ZLIB)
    find_package(ZLIB QUIET)
    if(ZLIB_FOUND)
        list(APPEND GFXPRIM_DEFINES HAVE_ZLIB)
        list(APPEND GFXPRIM_LIBRARIES ${ZLIB_LIBRARIES})
        list(APPEND GFXPRIM_INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS})
        message(STATUS "GFXprim: [x] zlib " ${ZLIB_VERSION_STRING})
    else()
        message(STATUS "GFXprim: [ ] zlib")
    endif()
endif()

if(NOT GFXPRIM_DISABLE_LIBARCHIVE)
    find_package(LibArchive QUIET)
    if(LibArchive_FOUND)
        list(APPEND GFXPRIM_DEFINES HAVE_LIBARCHIVE)
        list(APPEND GFXPRIM_LIBRARIES ${LibArchive_LIBRARIES})
        list(APPEND GFXPRIM_INCLUDE_DIRS ${LibArchive_INCLUDE_DIRS})
        message(STATUS "GFXprim: [x] libarchive " ${LibArchive_VERSION})
    else()
        message(STATUS "GFXprim: [ ] libarchive")
    endif()
endif()

if(NOT GFXPRIM_DISABLE_LIBXCB)
    find_package(XCB QUIET)
    if(XCB_FOUND)
        list(APPEND GFXPRIM_DEFINES HAVE_LIBXCB)
        list(APPEND GFXPRIM_LIBRARIES ${XCB_LIBRARIES})
        list(APPEND GFXPRIM_INCLUDE_DIRS ${XCB_INCLUDE_DIRS})
        list(APPEND GFXPRIM_COMPILE_OPTIONS ${XCB_DEFINITIONS})
        list(APPEND GFXPRIM_DEFINES HAVE_XCB_UTIL_ERRORS)
        message(STATUS "GFXprim: [x] xcb " ${XCB_VERSION})
    else()
        message(STATUS "GFXprim: [ ] xcb")
    endif()
endif()

if(NOT GFXPRIM_DISABLE_X_SHM)
    find_package(X11 QUIET)
    if(X11_XShm_FOUND)
        list(APPEND GFXPRIM_DEFINES HAVE_X_SHM)
        list(APPEND GFXPRIM_LIBRARIES ${X11_LIBRARIES})
        list(APPEND GFXPRIM_INCLUDE_DIRS ${X11_INCLUDE_DIR} ${X11_XShm_INCLUDE_PATH})
        message(STATUS "GFXprim: [x] x_shm")
    else()
        message(STATUS "GFXprim: [ ] x_shm")
    endif()
endif()

if(NOT GFXPRIM_DISABLE_AALIB)
    # TODO: Add AALib find_package support
    find_package(AALib QUIET)
    if(AALib_FOUND)
        list(APPEND GFXPRIM_DEFINES HAVE_AALIB)
        message(STATUS "GFXprim: [x] aalib " ${AALib_VERSION})
    else()
        message(STATUS "GFXprim: [ ] aalib")
    endif()
endif()

if(NOT GFXPRIM_DISABLE_FREETYPE)
    find_package(Freetype QUIET)
    if(FREETYPE_FOUND)
        list(APPEND GFXPRIM_DEFINES HAVE_FREETYPE HAVE_FONTCONFIG)
        list(APPEND GFXPRIM_LIBRARIES ${FREETYPE_LIBRARIES})
        list(APPEND GFXPRIM_INCLUDE_DIRS PUBLIC ${FREETYPE_INCLUDE_DIRS})
        message(STATUS "GFXprim: [x] freetype " ${FREETYPE_VERSION_STRING})
    else()
        message(STATUS "GFXprim: [ ] freetype")
    endif()
endif()

if(NOT GFXPRIM_DISABLE_JSON)
    find_package(json-c QUIET)
    if(json-c_FOUND)
        list(APPEND GFXPRIM_DEFINES HAVE_JSON_C)
        file(GLOB GFXPRIM_SOURCES_WIDGET ${GFXPRIM_SOURCE_DIR}/widgets/*.c)
        list(APPEND GFXPRIM_SOURCES_WIDGET)
        list(APPEND GFXPRIM_LIBRARIES json-c)
        message(STATUS "GFXprim: [x] json-c")
    else()
        message(STATUS "GFXprim: [ ] json-c")
    endif()
endif()

if(NOT GFXPRIM_DISABLE_DL)
    if(CMAKE_DL_LIBS)
        list(APPEND GFXPRIM_LIBRARIES ${CMAKE_DL_LIBS})
        list(APPEND GFXPRIM_DEFINES HAVE_DL)
        message(STATUS "GFXprim: [x] dl")
    else()
        message(STATUS "GFXprim: [ ] dl")
    endif()
endif()

if(NOT GFXPRIM_DISABLE_V4L2)
    if(UNIX)
        list(APPEND GFXPRIM_DEFINES HAVE_V4L2)
        message(STATUS "GFXprim: [x] v4l2")
    else()
        message(STATUS "GFXprim: [ ] v4l2")
    endif()
endif()

if(NOT GFXPRIM_DISABLE_PTHREAD)
    find_package(Threads QUIET)
    if(Threads_FOUND)
        list(APPEND GFXPRIM_LIBRARIES Threads::Threads)
        list(APPEND GFXPRIM_DEFINES HAVE_PTHREAD)
        message(STATUS "GFXprim: [x] pthread")
    else()
        message(STATUS "GFXprim: [ ] pthread")
    endif()
endif()

if(NOT GFXPRIM_DISABLE_BACKTRACE)
    find_package(Backtrace QUIET)
    if(Backtrace_FOUND)
        list(APPEND GFXPRIM_INCLUDE_DIRS ${Backtrace_INCLUDE_DIRS})
        list(APPEND GFXPRIM_LIBRARIES ${Backtrace_LIBRARIES})
        message(STATUS "GFXprim: [x] backtrace")
    else()
        message(STATUS "GFXprim: [ ] backtrace")
    endif()
endif()

# Config
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/config.h "// Empty config file generated by CMake.")

# Static Build
if(GFXPRIM_BUILD_STATIC)
    add_library(gfxprim-static STATIC ${GFXPRIM_SOURCES})
    target_include_directories(gfxprim-static PUBLIC ${GFXPRIM_INCLUDE_DIRS})
    target_compile_definitions(gfxprim-static PUBLIC ${GFXPRIM_DEFINES})
    if(GFXPRIM_LIBRARIES)
        target_link_libraries(gfxprim-static PRIVATE ${GFXPRIM_LIBRARIES})
    endif()
    if(GFXPRIM_COMPILE_OPTIONS)
        target_compile_options(gfxprim-static PUBLIC ${GFXPRIM_COMPILE_OPTIONS})
    endif()
endif()

# Shared Build
if(GFXPRIM_BUILD_SHARED)
    add_library(gfxprim SHARED ${GFXPRIM_SOURCES})
    target_include_directories(gfxprim PUBLIC ${GFXPRIM_INCLUDE_DIRS})
    target_compile_definitions(gfxprim PUBLIC ${GFXPRIM_DEFINES})
    if(GFXPRIM_LIBRARIES)
        target_link_libraries(gfxprim PRIVATE ${GFXPRIM_LIBRARIES})
    endif()
    if(GFXPRIM_COMPILE_OPTIONS)
        target_compile_options(gfxprim-static PUBLIC ${GFXPRIM_COMPILE_OPTIONS})
    endif()
endif()

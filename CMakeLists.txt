cmake_minimum_required(VERSION 3.16)
project(PitchShifter VERSION 0.0.1 DESCRIPTION "An vocal 300 like" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

## Path to source-code
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(CORE_DIR ${SRC_DIR}/core)
set(CLI_DIR ${SRC_DIR}/cli)

## Module
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

## Fonts
file(GLOB_RECURSE CORE_SOURCES ${CORE_DIR}/*.cpp ${CORE_DIR}/*.c)
set(SOURCES
    ${CLI_DIR}/main.cpp
    ${CORE_SOURCES}
)

find_package(Qt6 COMPONENTS Core QUIET)
if (NOT Qt6_FOUND)
    message(STATUS "Qt6 not found — trying Qt5...")
    find_package(Qt5 COMPONENTS Core REQUIRED)
    set(QT_LIBS Qt5::Core)
else()
    message(STATUS "Qt6 found!")
    set(QT_LIBS Qt6::Core)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(PORTAUDIO portaudio-2.0)

if (PORTAUDIO_FOUND)
    message(STATUS "Using PortAudio by pkg-config")
else()
    message(WARNING "pkg-config not found PortAudio — trying manually...")
    find_path(PORTAUDIO_INCLUDE_DIR portaudio.h)
    find_library(PORTAUDIO_LIBRARY NAMES portaudio)
endif()

if (NOT PORTAUDIO_INCLUDE_DIR AND NOT PORTAUDIO_INCLUDE_DIRS)
    message(FATAL_ERROR "PortAudio not found! Install with 'sudo apt install portaudio19-dev' or 'brew install portaudio'")
endif()


##################Linking#################################################
add_executable(${PROJECT_NAME} ${SOURCES})

## Include directories
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${SRC_DIR}
        ${PORTAUDIO_INCLUDE_DIRS}
        ${PORTAUDIO_INCLUDE_DIR}
)

## Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${QT_LIBS}
        ${PORTAUDIO_LIBRARIES}
        ${PORTAUDIO_LIBRARY}
)

## Install
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY ${SRC_DIR}/core/audio/
    DESTINATION include/pitchshifter/core/audio
    FILES_MATCHING PATTERN "*.h"
)
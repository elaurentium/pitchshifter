## Put logger on lib
add_subdirectory(logger)

## Collect audio fonts
file(GLOB_RECURSE CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/audio/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/audio/effects/*.cpp
)

## Includes public headers
target_include_directories(PitchShifterCore
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

## Link with Logger
target_link_libraries(PitchShifterCore
    PUBLIC
        Logger
)
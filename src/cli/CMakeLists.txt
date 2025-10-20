## Main Executable
add_executable(PitchShifter main.cpp)

## Link with core lib
target_link_libraries(PitchShifter PRIVATE PitchShifter::Core ${QT_LIBS})

## Include directories
target_include_directories(PitchShifter PRIVATE ${CMAKE_SOURCE_DIR}/src)

## Install
install(TARGETS PitchShifter RUNTIME DESTINATION bin)
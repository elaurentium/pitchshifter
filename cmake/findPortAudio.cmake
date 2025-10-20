find_path(PORTAUDIO_INCLUDE_DIR
    NAMES portaudio.h
    PATHS
        /usr/include
        /usr/local/include
        /opt/homebrew/include
)

find_library(PORTAUDIO_LIBRARY
    NAMES portaudio
    PATHS
        /usr/lib
        /usr/local/lib
        /opt/homebrew/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PortAudio
    REQUIRED_VARS PORTAUDIO_LIBRARY PORTAUDIO_INCLUDE_DIR
)

if(PORTAUDIO_FOUND AND NOT TARGET PortAudio::PortAudio)
    add_library(PortAudio::PortAudio UNKNOWN IMPORTED)
    set_target_properties(PortAudio::PortAudio PROPERTIES
        IMPORTED_LOCATION "${PORTAUDIO_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${PORTAUDIO_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(PORTAUDIO_INCLUDE_DIR PORTAUDIO_LIBRARY)
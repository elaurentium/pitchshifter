#ifndef PITCH_CONFIG_H
#define PITCH_CONFIG_H

#include <string>

#define APP_PREFIX "/usr/local"
#define SOURCE_DIR "/home/bacteriafield/pitchshifter"
#define BUILD_DIR "/home/bacteriafield/pitchshifter/build"

#define DSPENGINE_VERSION_MAJOR 1
#define DSPENGINE_VERSION_MINOR 0
#define DSPENGINE_VERSION_PATCH 0
#define DSPENGINE_VERSION "1.0.0"

#define DSPENGINE_GIT_REVISION "70419e5"

#define MAX_BUFFER_SIZE 4096
#define MAX_FX 8

/* #undef HAVE_LIBSNDFILE */
/* #undef HAVE_RUBBERBAND */
/* #undef HAVE_JACK */
/* #undef HAVE_PORTAUDIO */
/* #undef HAVE_ALSA */

#ifndef DSPENGINE_HAVE_DEBUG
/* #undef DSPENGINE_HAVE_DEBUG */
#endif

#endif // PITCH_CONFIG_H

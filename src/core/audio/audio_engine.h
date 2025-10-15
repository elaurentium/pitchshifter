/*
    MIT License

    Copyright (c) 2025 Evandro

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

*/

#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include <limits>
#include <sstream>

#include <cassert>
#include <chrono>
#include <deque>
#include <QString>
#include <queue>
#include <memory>
#include <mutex>
#include <thread>

#define AUDIO_ENGINE_DEBUG 0

typedef int (*audioProcessCallBack)(uint32_t, void *);

namespace PCore {
    class AudioEngine {
        public:
            enum class state {
                // Not even the constructor has been called
                Uninitialized = 1,
                // Not ready, but most pointers are now valid or NULL
                Initialized = 2,
                // Drivers are set up, but not ready to process audio.
                Prepared = 3,
                // Ready to process audio
                Ready = 4,
                // Transport rolling yet. But not ready to process audio
                CountIn = 5,
                // Transport is rolling.
                Playing = 6,
            };

            static QString StateToQString(const state& state);
    };
};


#endif // AUDIO_ENGINE_H
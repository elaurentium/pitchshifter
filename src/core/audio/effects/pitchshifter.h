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

#ifndef PITCHSHIFTER_H
#define PITCHSHIFTER_H

#include "vocal_effect.h"

namespace PCore {
    class PitchShifter : public VocalEffect {
        private:
            std::vector<float> buffer1, buffer2, window;
            int writePos, bufferSize;
            float readPos1, readPos2;
            float pitchRatio, crossfade;
            float formantShift, mix;

        public:
            PitchShifter(int sampleRate);
            void process(std::vector<float> &buffer, int sampleRate);
            void setParameters(const std::string &param, float value);
            float getLatency() const;
    };
}

#endif // PITCHSHIFTER_H
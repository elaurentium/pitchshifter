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

#ifndef FLANGER_H
#define FLANGER_H

#include <vector>
#include <cmath>
#include <algorithm>

#include "../audio_node.h"

namespace PCore { 
    class Flanger : public AudioNode {
        private:
            int   sampleRate_ = 44100;
            float lfoPhase_   = 0.0f;    // [rad]
            float lfoRate_    = 0.25f;   // Hz (rate)
            float depthSec_   = 0.0025f; // depth in secons (depth) ~2.5ms
            float feedback_   = 0.25f;   // 0..0.95
            float mix_        = 0.35f;   // 0..1
            float baseDelaySec_ = 0.001f; // delay base ~1 ms

            // Estado
            std::vector<float> dBuffer_; // buffer
            size_t writePos_ = 0;

            // Helper
            inline size_t secToSamples(float sec) const {
                return static_cast<size_t>(std::max(1, (int)std::lround(sec * sampleRate_)));
            }

        public:
            explicit Flanger(int sampleRate);
            void prepare(int sr, int block, int inCh, int outCh) override;
            void process(const float* const* in, float* const* out, unsigned long frames) override;
            void setParameters(const std::string &param, float value);

    };
}

#endif // FLANGER_H
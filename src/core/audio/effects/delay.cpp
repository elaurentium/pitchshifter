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

#include <cmath>
#include <algorithm>

#include "delay.h"

namespace PCore {
    Delay::Delay(int sampleRate) : sampleRate_(sampleRate) {}

    void Delay::prepare(int sr, int block, int inCh, int outCh) {
        sampleRate_ = sr;
        size_t delaySamples = static_cast<size_t>(std::max(1, (int)std::lround(dTimeMs_ * 0.001 * sampleRate_)));
        dBuffer_.assign(delaySamples, 0.0f);
        writePos_ = 0;
    }

    void Delay::process(const float* const* in, float* const* out, unsigned long frames) {
        if (!out || !out[0]) return;

        const float* x = (in && in[0]) ? in[0] : nullptr;
        float* y = out[0];

        if (!x) { std::fill_n(y, frames, 0.0f); return; }

        const size_t delayLen = dBuffer_.size();
        if (delayLen == 0) { std::copy(x, x + frames, y); return; }

        size_t wp = static_cast<size_t>(writePos_);
        for (unsigned long i = 0; i < frames; ++i) {
            float d = dBuffer_[wp];
            float v = x[i] + feedback_ * d;        // feedback
            dBuffer_[wp] = v;
            y[i] = (1.0f - mix_) * x[i] + mix_ * d; // mix signal dry + signal wet
            wp = (wp + 1) % delayLen;
        }
        writePos_ = static_cast<int>(wp);
    }

    void Delay::setParameters(const std::string& param, float value) {
        if (param == "time_ms") {
            dTimeMs_ = std::max(1, (int)std::lround(value));
            size_t delaySamples = static_cast<size_t>(std::max(1, (int)std::lround(dTimeMs_ * 0.001 * sampleRate_)));
            dBuffer_.assign(delaySamples, 0.0f);
            writePos_ = 0;
        } else if (param == "feedback") {
            feedback_ = std::clamp(value, 0.0f, 0.95f);
        } else if (param == "mix") {
            mix_ = std::clamp(value, 0.0f, 1.0f);
        }
    }
};
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

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <memory>

#include "chorus.h"

namespace PCore {
    Chorus::Chorus(int sampleRate) : writePos(0), lfoPhase(0.0f),
                            lfoRate(2.0f), depth(0.003f), mix(0.5f) {
        dBuffer.resize(sampleRate * 0.5f, 0.0f); // 50ms buffer
        baseDelay = sampleRate * 0.01f; // 10ms base delay
    }

    void Chorus::process(std::vector<float> &buffer, int sampleRate) {
        float lfoInc = (2.0f * M_PI * lfoRate) / sampleRate;

        for (size_t i = 0; i < buffer.size(); i++) {
            float input = buffer[i];

            // LFO modulation
            float lfo = std::sin(lfoPhase);
            lfoPhase += lfoInc;
            
            if (lfoPhase > 2.0f * M_PI) lfoPhase -= 2.0f * M_PI;

            // Chorus effect -> modulate delay time
            float modDelay = baseDelay + lfo * depth * sampleRate;
            int readPos = static_cast<int>(writePos - modDelay + dBuffer.size()) % dBuffer.size();

            float delayed = dBuffer[readPos];
            dBuffer[writePos] = input;
            writePos = (writePos + 1) % dBuffer.size();

            buffer[i] = input * (1.0f - mix) + (input + delayed) * 0.5f * mix;
        }
    }

    void Chorus::setParameters(const std::string &param, float value) {
        if (param == "rate") lfoRate = std::clamp(value, 0.1f, 10.0f);
        else if (param == "depth") depth = std::clamp(value, 0.0f, 0.01f);
        else if (param == "mix") mix = std::clamp(value, 0.0f, 1.0f);
    }
};
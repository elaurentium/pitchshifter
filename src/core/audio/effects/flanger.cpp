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

#include "flanger.h"

namespace PCore {
    Flanger::Flanger(int sampleRate) : writePos(0), lfoPhase(0.0f), 
                                   lfoRate(0.5f), depth(0.002f), 
                                   feedback(0.7f), mix(0.5f) {
        dBuffer.resize(sampleRate * 0.2f, 0.0f);
    }

    void Flanger::process(std::vector<float> &buffer, int sampleRate) {
        float lfoInc = (2.0f * M_PI * lfoRate) / sampleRate;

        for (size_t i = 0; i < buffer.size(); i++) {
            float input = buffer[i];

            float lfo = std::sin(lfoPhase);
            lfoPhase += lfoInc;
            if (lfoPhase > 2.0f * M_PI) lfoPhase -= 2.0f * M_PI;

            float modDelay = (1.0f + lfo) * depth * sampleRate;

            int readPos = static_cast<int>(writePos - modDelay + dBuffer.size()) % dBuffer.size();

            float delayed = dBuffer[readPos];
            dBuffer[writePos] = input;
            writePos = (writePos + 1) % dBuffer.size();

            buffer[i] = input * (1.0f - mix) + delayed * mix;
        }
    }

    void Flanger::setParameters(const std::string& param, float value) {
        if (param == "rate") lfoRate = std::clamp(value, 0.1f, 5.0f);
        else if (param == "depth") depth = std::clamp(value, 0.0f, 0.005f);
        else if (param == "feedback") feedback = std::clamp(value, 0.0f, 0.95f);
        else if (param == "mix") mix = std::clamp(value, 0.0f, 1.0f);
    }
}
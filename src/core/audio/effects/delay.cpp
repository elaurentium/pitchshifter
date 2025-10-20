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

#include "delay.h"

namespace PCore {
    Delay::Delay(int sampleRate) : writePos(0), feedback(0.5f), mix(0.3f) {
        int maxDelay = sampleRate * 2; // 2 seconds max delay
        dBuffer.resize(maxDelay, 0.0f);
        dTime = sampleRate * 0.375f; // 375ms default
    }

    void Delay::process(std::vector<float> &buffer, int sampleRate) {
        for (size_t i = 0; i < buffer.size(); i++) {
            float input = buffer[i];
            int readPos = (writePos - dTime + dBuffer.size()) % dBuffer.size();
            float delayed = dBuffer[readPos];

            dBuffer[writePos] = input + delayed * feedback;
            writePos = (writePos + 1) % dBuffer.size();

            buffer[i] = input * (1.0f - mix) + delayed * mix;
        }
    }

    void Delay::setParameters(const std::string &param, float value) {
        if (param == "time") dTime = static_cast<int>(value);
        if (param == "feedback") feedback = std::clamp(value, 0.0f, 0.95f);
        if (param == "mix") mix = std::clamp(value, 0.0f, 1.0f);
    }
};
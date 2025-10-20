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

#include <algorithm>

#include "reverb.h"

namespace PCore {
    Reverb::Reverb(int sampleRate) : writePos(0), decay(0.5f), mix(0.3f) {
        dLenght = sampleRate * 0.5f;
        dBuffer.resize(dLenght * 4, 0.0f);
    }

    void Reverb::process(std::vector<float> &buffer, int sampleRate) {
        for (size_t i = 0; i < buffer.size(); i++) {
            float input = buffer[i];

            // Multiply delay taps for reverb effect
            float delayd1 = dBuffer[(writePos - dLenght) & (dBuffer.size() - 1)];
            float delayd2 = dBuffer[(writePos - dLenght * 2) & (dBuffer.size() - 1)];
            float delayd3 = dBuffer[(writePos - dLenght * 3) & (dBuffer.size() - 1)];

            float reverbSignal = (delayd1 + delayd2 * 0.7f + delayd3 * 0.5f) / 3.0f;

            dBuffer[writePos] = input + reverbSignal * decay;
            writePos = (writePos + 1) % dBuffer.size();

            buffer[i] = input * (1.0f - mix) + reverbSignal * mix;
        }
    }

    void Reverb::setParameters(const std::string &param, float value) {
        if (param == "decay") decay = std::clamp(value, 0.0f, 0.99f);
        if (param == "mix") mix = std::clamp(value, 0.0f, 1.0f);
    }
};
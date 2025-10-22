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

#include "pitchshifter.h"

// TODO: learn how to performated pitchshifting cuz sounds weird asf
namespace PCore {
    PitchShifter::PitchShifter(int sampleRate) : writePos(0), readPos1(0), readPos2(0),
                                             pitchRatio(1.0f), crossfade(0.0f) {
        bufferSize = sampleRate * 0.01f;
        buffer1.resize(bufferSize, 0.0f);
        buffer2.resize(bufferSize, 0.0f);
        readPos2 = bufferSize / 2;
    }

    void PitchShifter::process(std::vector<float> &buffer, int sampleRate) {
        for (size_t i = 0; i < buffer.size(); i++) {
            float input = buffer[i];
            
            buffer1[writePos] = input;
            buffer2[writePos] = input;
            
            int r1 = static_cast<int>(readPos1);
            int r2 = static_cast<int>(readPos2);
            float frac1 = readPos1 - r1;
            float frac2 = readPos2 - r2;
            
            float sample1 = buffer1[r1] * (1.0f - frac1) + buffer1[(r1 + 1) % bufferSize] * frac1;
            float sample2 = buffer2[r2] * (1.0f - frac2) + buffer2[(r2 + 1) % bufferSize] * frac2;
            
            float cf = (std::sin(crossfade * M_PI) + 1.0f) * 0.5f;
            buffer[i] = sample1 * (1.0f - cf) + sample2 * cf;
            
            readPos1 += pitchRatio;
            readPos2 += pitchRatio;
            crossfade += 0.01f;
            
            if (readPos1 >= bufferSize) readPos1 -= bufferSize;
            if (readPos2 >= bufferSize) readPos2 -= bufferSize;
            if (crossfade >= 2.0f) crossfade -= 2.0f;
            
            writePos = (writePos + 1) % bufferSize;
        }
    }

    void PitchShifter::setParameters(const std::string &param, float value) {
        if (param == "pitch") {
            pitchRatio = std::pow(2.0f, value / 12.0f);
        }
    }
}
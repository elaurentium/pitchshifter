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
#include "pitchshifter.h"

// TODO: learn how to performated pitchshifter works cuz sounds weird asf
namespace PCore {
    PitchShifter::PitchShifter(int sampleRate) : writePos(0), readPos1(0), readPos2(0),
                                             pitchRatio(1.0f), crossfade(0.0f),
                                             formantShift(1.0f), mix(1.0f) {
        // Buffer size: 50ms for quality pitch shifting
        bufferSize = static_cast<int>(sampleRate * 0.05f);
        buffer1.resize(bufferSize, 0.0f);
        buffer2.resize(bufferSize, 0.0f);
        window.resize(bufferSize);

        // Initialize read positions with 50% offset for smooth crossfading
        readPos2 = bufferSize * 0.5f;
        
        // Pre-calculate Hann window for artifact reduction
        for (int i = 0; i < bufferSize; i++) {
            window[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / bufferSize));
        } 
    }

    void PitchShifter::process(std::vector<float> &buffer, int sampleRate) {
        // Adaptive crossfade speed based on buffer size
        const float crossfadeSpeed = 2.0f / bufferSize;
        
        for (size_t i = 0; i < buffer.size(); i++) {
            const float input = buffer[i];
            
            // Write input to both circular buffers
            buffer1[writePos] = input;
            buffer2[writePos] = input;
            
            // Calculate read positions with fractional part for interpolation
            const int r1 = static_cast<int>(readPos1);
            const int r2 = static_cast<int>(readPos2);
            const float frac1 = readPos1 - r1;
            const float frac2 = readPos2 - r2;
            
            // Linear interpolation with windowing for smooth transitions
            const int r1Next = (r1 + 1) % bufferSize;
            const int r2Next = (r2 + 1) % bufferSize;
            
            const float interp1 = buffer1[r1] * (1.0f - frac1) + buffer1[r1Next] * frac1;
            const float interp2 = buffer2[r2] * (1.0f - frac2) + buffer2[r2Next] * frac2;
            
            // Apply Hann window to reduce spectral artifacts
            const float sample1 = interp1 * window[r1];
            const float sample2 = interp2 * window[r2];
            
            // Equal-power crossfade for constant energy
            const float cf = std::clamp(crossfade * 0.5f, 0.0f, 1.0f);
            const float gain1 = std::cos(cf * M_PI * 0.5f);
            const float gain2 = std::sin(cf * M_PI * 0.5f);
            
            const float shifted = sample1 * gain1 + sample2 * gain2;
            
            // Mix dry/wet signal
            buffer[i] = input * (1.0f - mix) + shifted * mix;
            
            // Advance read positions based on pitch ratio
            readPos1 += pitchRatio;
            readPos2 += pitchRatio;
            crossfade += crossfadeSpeed;
            
            // Wrap read positions and crossfade
            if (readPos1 >= bufferSize) readPos1 -= bufferSize;
            if (readPos2 >= bufferSize) readPos2 -= bufferSize;
            if (crossfade >= 2.0f) crossfade -= 2.0f;
            
            // Advance write position
            writePos = (writePos + 1) % bufferSize;
        }
    }

    void PitchShifter::setParameters(const std::string &param, float value) {
        if (param == "pitch") {
            // Convert semitones to pitch ratio (12-TET)
            pitchRatio = std::pow(2.0f, value / 12.0f);
            
            // Auto-adjust formant shift for natural sound
            // Positive pitch shift = shorter vocal tract (younger/feminine)
            // Negative pitch shift = longer vocal tract (older/masculine)
            formantShift = std::pow(2.0f, -value / 24.0f); // Half the pitch shift amount
            
        } else if (param == "formant") {
            // Manual formant control (-12 to +12 semitones)
            formantShift = std::pow(2.0f, value / 12.0f);
            
        } else if (param == "mix") {
            // Dry/wet mix (0.0 = dry, 1.0 = wet)
            mix = std::clamp(value, 0.0f, 1.0f);
        }
    }

    float PitchShifter::getLatency() const {
        // Return latency in samples for DAW compensation
        return bufferSize * 0.5f;
    }
}
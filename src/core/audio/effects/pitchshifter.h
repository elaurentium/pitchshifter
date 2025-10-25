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

#include "../audio_node.h"

#include <vector>
#include <string>

namespace PCore {
    class PitchShifter : public AudioNode {
        private:
            // Parameters
			int   sampleRate_   = 44100;
			float pitchRatio_   = 1.0f;   // scalar ratio from semitones
			float formantRatio_ = 1.0f;   // placeholder (not applied in this time-domain shifter)
			float mix_          = 1.0f;   // 0..1 wet

			// Windowed dual-buffer state
			std::vector<float> buffer1_;
			std::vector<float> buffer2_;
			std::vector<float> window_;   // Hann
			int    bufferSize_ = 0;       // in samples
			int    writePos_   = 0;
			float  readPos1_   = 0.0f;
			float  readPos2_   = 0.0f;
			float  crossfade_  = 0.0f;    // 0..2.0 cycles

			// Helpers
			void rebuildWindow();         // rebuild Hann window for current bufferSize_
			inline float hannAt(int idx) const;


        public:
            explicit PitchShifter(int sampleRate);

			// AudioNode API
			void prepare(int sr, int block, int inCh, int outCh) override;
			void process(const float* const* in, float* const* out, unsigned long frames) override;

			// Parameters (public API)
			void setParameters(const std::string& param, float value);

			// Latency in samples (useful for DAW compensation or alignment)
			int latencySamples() const;
    };
}

#endif // PITCHSHIFTER_H
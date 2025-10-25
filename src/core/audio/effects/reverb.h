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

#ifndef REVERB_H
#define REVERB_H

#include "../audio_node.h"

#include <cstddef>
#include <vector>
#include <string>

namespace PCore {
    class Reverb : public AudioNode {
        private:
		    //State
            std::vector<float> dBuffer_;
            size_t writePos_ = 0;
            
			// Params
			int   sampleRate_ = 44100;
			float decay_      = 0.5f;   // 0..0.99
			float mix_        = 0.3f;   // 0..1
			float baseTime_   = 0.35f;  // time base s (to tap 1)
			// Relation between taps (on multi)
			float t2mul_      = 2.0f;
			float t3mul_      = 3.0f;

			// Helpers
			inline size_t secToSamples(float sec) const {
				return static_cast<size_t>(std::max(1, (int)std::lround(sec * sampleRate_)));
			}

        public:
            explicit Reverb(int sampleRate);
			void prepare(int sr, int block, int inCh, int outCh) override;
            void process(const float* const* in, float* const* out, unsigned long frames) override;
            void setParameters(const std::string &param, float value);
    };
}

#endif // REVERB_H
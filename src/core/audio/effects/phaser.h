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

#ifndef PCORE_EFFECTS_PHASER_H
#define PCORE_EFFECTS_PHASER_H

#include "../audio_node.h"
#include <vector>
#include <string>

namespace PCore {
    class Phaser : public AudioNode {
    public:
        Phaser(int sampleRate);
        virtual ~Phaser() = default;

        void prepare(int sampleRate, int maxBlock, int inChans, int outChans) override;
        void process(const float* const* in, float* const* out, unsigned long frames) override;

        void setParameters(const std::string& param, float value);

    private:
        int sampleRate_;
        
        // Settings
        float rate_ = 0.5f;   // LFO rate in Hz
        float depth_ = 0.7f;  // LFO depth 0..1
        float feedback_ = 0.5f; // Feedback amount 0..1
        float mix_ = 0.5f;    // Dry/Wet mix 0..1
        
        // All-pass filters (6 stages is common)
        static const int NUM_STAGES = 6;
        
        struct AllPass {
            float z1 = 0.0f;
            float process(float x, float al) {
                // y[n] = al * x[n] + x[n-1] - al * y[n-1]
                // Transposed form:
                // y = al * (x - z1) + z1
                // z1 = y (actually need correct structure)
                
                // Diff eq: y(n) = C*x(n) + x(n-1) - C*y(n-1) where C is coeff
                // Let's use standard direct form II or similar
                
                // y = C*x + z1;
                // z1 = x - C*y;
                
                float y = al * x + z1;
                z1 = x - al * y;
                // Anti-denormal happens here if needed
                return y;
            }
        };

        struct ChannelState {
            AllPass stages[NUM_STAGES];
            float lfoPhase = 0.0f;
            float lastOutput = 0.0f; // For feedback
        };
        
        std::vector<ChannelState> channels_;
    };
}

#endif // PCORE_EFFECTS_PHASER_H

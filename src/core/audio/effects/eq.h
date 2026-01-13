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

#ifndef PCORE_EFFECTS_EQ_H
#define PCORE_EFFECTS_EQ_H

#include "../audio_node.h"
#include <vector>
#include <string>

namespace PCore {
    // Simple 3-band EQ using Biquad filters
    class Eq : public AudioNode {
    public:
        Eq(int sampleRate);
        virtual ~Eq() = default;

        void prepare(int sampleRate, int maxBlock, int inChans, int outChans) override;
        void process(const float* const* in, float* const* out, unsigned long frames) override;

        void setParameters(const std::string& param, float value);

    private:
        struct Biquad {
            float b0 = 0, b1 = 0, b2 = 0;
            float a1 = 0, a2 = 0;
            float z1 = 0, z2 = 0;

            void processBlock(const float* in, float* out, unsigned long frames);
            void reset() { z1 = 0; z2 = 0; }
        };

        int sampleRate_;

        // Gains in dB
        float lowGain_ = 0.0f;
        float midGain_ = 0.0f;
        float highGain_ = 0.0f;

        // Crossover/center frequencies
        float lowFreq_ = 100.0f;  // Low Shelf cutoff
        float midFreq_ = 1000.0f; // Peaking center
        float highFreq_ = 5000.0f; // High Shelf cutoff
        
        // Q factor for mid peaking
        float midQ_ = 0.707f;

        // Per-channel filters: 3 filters per channel (Low, Mid, High)
        // Stored as channel -> vector of 3 Biquads
        std::vector<std::vector<Biquad>> filters_;

        void recalculateCoefficients();
        void calculateLowShelf(Biquad& f, float freq, float currentGainDB);
        void calculatePeaking(Biquad& f, float freq, float Q, float currentGainDB);
        void calculateHighShelf(Biquad& f, float freq, float currentGainDB);
    };
}

#endif // PCORE_EFFECTS_EQ_H

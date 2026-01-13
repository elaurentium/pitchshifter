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

#ifndef PCORE_EFFECTS_COMPRESSOR_H
#define PCORE_EFFECTS_COMPRESSOR_H

#include "../audio_node.h"
#include <vector>
#include <string>

namespace PCore {
    class Compressor : public AudioNode {
    public:
        Compressor(int sampleRate);
        virtual ~Compressor() = default;

        void prepare(int sampleRate, int maxBlock, int inChans, int outChans) override;
        void process(const float* const* in, float* const* out, unsigned long frames) override;

        void setParameters(const std::string& param, float value);

    private:
        int sampleRate_;
        float threshold_ = -20.0f; // dB
        float ratio_ = 4.0f;
        float attackMs_ = 10.0f;
        float releaseMs_ = 100.0f;
        float makeupGain_ = 0.0f; // dB

        // Runtime calculated coefficients
        float attackCoeff_ = 0.0f;
        float releaseCoeff_ = 0.0f;

        // Per-channel envelope state
        std::vector<float> envelope_;
        
        void recalculateCoefficients();
    };
}

#endif // PCORE_EFFECTS_COMPRESSOR_H

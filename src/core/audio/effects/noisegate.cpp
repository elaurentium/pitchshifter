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

#include "noisegate.h"
#include <cmath>
#include <algorithm>

namespace PCore {

    NoiseGate::NoiseGate(int sampleRate) : sampleRate_(sampleRate) {
        recalculateCoefficients();
    }

    void NoiseGate::prepare(int sampleRate, int maxBlock, int inChans, int outChans) {
        sampleRate_ = sampleRate;
        int channels = std::max(inChans, outChans);
        envelope_.assign(channels, 0.0f);
        gain_.assign(channels, 0.0f);
        recalculateCoefficients();
    }

    void NoiseGate::recalculateCoefficients() {
        if (sampleRate_ <= 0) return;
        
        float attSec = std::max(0.001f, attackMs_) * 0.001f;
        float relSec = std::max(0.001f, releaseMs_) * 0.001f;

        attackCoeff_ = std::exp(-1.0f / (attSec * sampleRate_));
        releaseCoeff_ = std::exp(-1.0f / (relSec * sampleRate_));
    }

    void NoiseGate::setParameters(const std::string& param, float value) {
        if (param == "threshold") {
            threshold_ = value; // dB
        } else if (param == "attack") {
            attackMs_ = std::max(0.1f, value);
            recalculateCoefficients();
        } else if (param == "release") {
            releaseMs_ = std::max(1.0f, value);
            recalculateCoefficients();
        }
    }

    void NoiseGate::process(const float* const* in, float* const* out, unsigned long frames) {
        if (!out || !out[0]) return;
        
        if (!in || !in[0]) {
            std::fill_n(out[0], frames, 0.0f);
            return;
        }

        size_t numChannels = envelope_.size();
        float thresholdLinear = std::pow(10.0f, threshold_ / 20.0f);
        
        for (size_t c = 0; c < numChannels; ++c) {
            if (!in[c] || !out[c]) break;

            const float* inp = in[c];
            float* outp = out[c];
            float& env = envelope_[c];
            float& g = gain_[c];

            for (unsigned long i = 0; i < frames; ++i) {
                float inputSample = inp[i];
                float absInput = std::fabs(inputSample);

                // Simple envelope follower with attack/release
                if (absInput > env) {
                    env = attackCoeff_ * env + (1.0f - attackCoeff_) * absInput;
                } else {
                    env = releaseCoeff_ * env + (1.0f - releaseCoeff_) * absInput;
                }
                
                // Target gain: 1.0 if above threshold, 0.0 if below
                // Soft knee could be added, but hard knee is standard for simple gates
                float targetGain = (env > thresholdLinear) ? 1.0f : 0.0f;
                
                // Smooth gain transition
                if (targetGain > g) {
                     // Opening (Attack)
                     g = attackCoeff_ * g + (1.0f - attackCoeff_) * targetGain;
                } else {
                     // Closing (Release)
                     g = releaseCoeff_ * g + (1.0f - releaseCoeff_) * targetGain;
                }

                outp[i] = inputSample * g;
            }
        }
    }
}

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

#include "compressor.h"
#include <cmath>
#include <algorithm>
#include <iostream> // For logging if needed

namespace PCore {

    Compressor::Compressor(int sampleRate) : sampleRate_(sampleRate) {
        recalculateCoefficients();
    }

    void Compressor::prepare(int sampleRate, int maxBlock, int inChans, int outChans) {
        sampleRate_ = sampleRate;
        // Allocate space for envelope state for each input channel
        // If inChanned > outChannels, we might process redundant envelopes, but that's fine.
        // Usually we process N input channels to N output channels.
        envelope_.assign(std::max(inChans, outChans), 0.0f);
        
        recalculateCoefficients();
    }

    void Compressor::recalculateCoefficients() {
        if (sampleRate_ <= 0) return;
        
        // Simple 1-pole coeff: exp(-1 / (time * fs))
        // Attack/Release are in ms
        float attSec = std::max(0.001f, attackMs_) * 0.001f;
        float relSec = std::max(0.001f, releaseMs_) * 0.001f;

        attackCoeff_ = std::exp(-1.0f / (attSec * sampleRate_));
        releaseCoeff_ = std::exp(-1.0f / (relSec * sampleRate_));
    }

    void Compressor::setParameters(const std::string& param, float value) {
        if (param == "threshold") {
            threshold_ = value; // dB
        } else if (param == "ratio") {
            ratio_ = std::max(1.0f, value);
        } else if (param == "attack") {
            attackMs_ = std::max(0.1f, value);
            recalculateCoefficients();
        } else if (param == "release") {
            releaseMs_ = std::max(1.0f, value);
            recalculateCoefficients();
        } else if (param == "makeup") {
            makeupGain_ = value; // dB
        }
    }

    void Compressor::process(const float* const* in, float* const* out, unsigned long frames) {
        if (!out || !out[0]) return;
        
        // If no input, silence output
        if (!in || !in[0]) {
            // Silence all output channels
            // (Assuming we know how many out channels from previous calls or just silence first one)
            // Ideally we should know numOutChannels. But 'prepare' is where we set it.
            // For safety, let's just assumes 1 or 2 typically. 
            // In a strict graph we would loop over all outputs. 
            // Here we assume out[0] is valid as per check above.
            std::fill_n(out[0], frames, 0.0f);
            return;
        }

        // Linear makeup gain from dB
        float makeupLinear = std::pow(10.0f, makeupGain_ / 20.0f);
        float thresholdLinear = std::pow(10.0f, threshold_ / 20.0f); // Not used directly in log domain calc usually

        // We process each channel independently (dual mono) or link them?
        // Let's do simple independent processing for now as per vocal usage
        
        // We need to know how many channels to process. 
        // Since the interface passes float* const* out, we don't strictly know N channels here 
        // without storing it from prepare. Let's assume we process up to envelope_.size().
        // But the caller might only provide valid pointers for some.
        // We'll trust the caller passes valid pointers for at least what we prepared for.
        
        size_t numChannels = envelope_.size();
        
        for (size_t c = 0; c < numChannels; ++c) {
            if (!in[c] || !out[c]) break; // Safety check

            const float* inp = in[c];
            float* outp = out[c];
            float& env = envelope_[c];

            for (unsigned long i = 0; i < frames; ++i) {
                float inputSample = inp[i];
                float absInput = std::fabs(inputSample);

                // Envelope follower
                if (absInput > env) {
                    env = attackCoeff_ * env + (1.0f - attackCoeff_) * absInput;
                } else {
                    env = releaseCoeff_ * env + (1.0f - releaseCoeff_) * absInput;
                }

                // Gain reduction calculation
                // Convert envelope to dB
                // Avoid log(0)
                float envdB = 20.0f * std::log10(std::max(1e-6f, env));
                
                float gainReductiondB = 0.0f;
                
                // If signal is above threshold
                if (envdB > threshold_) {
                    float slope = 1.0f - (1.0f / ratio_);
                    gainReductiondB = -slope * (envdB - threshold_);
                }

                // Convert GR to linear
                float gainLinear = std::pow(10.0f, gainReductiondB / 20.0f);
                
                // Apply gain + makeup
                outp[i] = inputSample * gainLinear * makeupLinear;
            }
        }
    }

}

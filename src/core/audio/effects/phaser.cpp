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

#include "phaser.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace PCore {

    Phaser::Phaser(int sampleRate) : sampleRate_(sampleRate) {}

    void Phaser::prepare(int sampleRate, int maxBlock, int inChans, int outChans) {
        sampleRate_ = sampleRate;
        channels_.assign(std::max(inChans, outChans), ChannelState());
    }

    void Phaser::setParameters(const std::string& param, float value) {
        if (param == "rate") rate_ = value;
        else if (param == "depth") depth_ = std::clamp(value, 0.0f, 1.0f);
        else if (param == "feedback") feedback_ = std::clamp(value, 0.0f, 0.95f); // limit feedback
        else if (param == "mix") mix_ = std::clamp(value, 0.0f, 1.0f);
    }

    void Phaser::process(const float* const* in, float* const* out, unsigned long frames) {
        if (!out || !out[0]) return;
        if (!in || !in[0]) {
            std::fill_n(out[0], frames, 0.0f);
            return;
        }

        size_t numC = channels_.size();
        
        // LFO constants
        const float lfoInc = 1.0f * rate_ / sampleRate_;
        const float lfoMin = 200.0f;
        const float lfoMax = 2000.0f; // Frequency sweep range

        for (size_t c = 0; c < numC; ++c) {
            ChannelState& st = channels_[c];
            const float* inp = in[c];
            float* outp = out[c];
            
            // If pointers invalid for this channel, skip
            if (!inp || !outp) break;

            for (unsigned long i = 0; i < frames; ++i) {
                // Update LFO
                st.lfoPhase += lfoInc;
                if (st.lfoPhase >= 1.0f) st.lfoPhase -= 1.0f;
                
                // Triangle LFO 0..1
                float lfo = (st.lfoPhase < 0.5f) ? (2.0f * st.lfoPhase) : (2.0f * (1.0f - st.lfoPhase));
                
                // Sweep frequency
                float freq = lfoMin + (lfoMax - lfoMin) * lfo * depth_; // simple linear sweep
                
                // Calculate allpass coefficient
                // alpha = (tan(w0/2) - 1) / (tan(w0/2) + 1) ... actually
                // alpha = (1 - sin(w0)) / cos(w0) is a shelf...
                // Allpass coeff C = (tan(pi*f/fs) - 1) / (tan(pi*f/fs) + 1)
                
                float w = M_PI * freq / sampleRate_;
                // approximate tan for small w? no, use std::tan. 
                // but for optimizing, can compute once per block if rate is low? 
                // Phaser sounds better with sample-accurate or small block updates.
                // Let's do per sample, w is small.
                float t = std::tan(w);
                float alpha = (t - 1.0f) / (t + 1.0f);

                float x = inp[i];
                float y = x + st.lastOutput * feedback_;
                
                // Run through allpass stages
                for (int s = 0; s < NUM_STAGES; ++s) {
                    y = st.stages[s].process(y, alpha);
                }
                
                st.lastOutput = y; // Feed filtered signal back

                // Mix
                outp[i] = x * (1.0f - mix_) + y * mix_;
            }
        }
    }
}

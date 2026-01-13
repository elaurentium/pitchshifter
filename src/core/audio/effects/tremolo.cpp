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

#include "tremolo.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace PCore {

    Tremolo::Tremolo(int sampleRate) : sampleRate_(sampleRate) {}

    void Tremolo::prepare(int sampleRate, int maxBlock, int inChans, int outChans) {
        sampleRate_ = sampleRate;
        channels_.assign(std::max(inChans, outChans), ChannelState());
        // Desync channels slightly? Or sync? Typically tremolo is sync unless stereo trem.
        // Let's keep them sync for now.
    }

    void Tremolo::setParameters(const std::string& param, float value) {
        if (param == "rate") rate_ = value;
        else if (param == "depth") depth_ = std::clamp(value, 0.0f, 1.0f);
        else if (param == "waveform") waveform_ = static_cast<int>(value);
    }

    void Tremolo::process(const float* const* in, float* const* out, unsigned long frames) {
        if (!out || !out[0]) return;
        if (!in || !in[0]) {
            std::fill_n(out[0], frames, 0.0f);
            return;
        }

        size_t numC = channels_.size();
        float inc = rate_ / sampleRate_;

        for (size_t c = 0; c < numC; ++c) {
            ChannelState& st = channels_[c];
            const float* inp = in[c];
            float* outp = out[c];
            
            if (!inp || !outp) break;

            for (unsigned long i = 0; i < frames; ++i) {
                // Update phase
                st.phase += inc;
                if (st.phase >= 1.0f) st.phase -= 1.0f;

                float lfo = 0.0f;
                // Generate LFO 0..1
                switch (waveform_) {
                    case 0: // Sine
                        lfo = 0.5f * (1.0f + std::sin(2.0f * M_PI * st.phase));
                        break;
                    case 1: // Triangle
                        lfo = (st.phase < 0.5f) ? (2.0f * st.phase) : (2.0f * (1.0f - st.phase));
                        break;
                    case 2: // Square
                        lfo = (st.phase < 0.5f) ? 1.0f : 0.0f;
                        break;
                    default: 
                        lfo = 0.0f; 
                        break;
                }
                
                // Modulate amplitude
                // Gain = 1 - depth + depth * lfo   ... or similar
                // If depth = 1, gain goes 0..1
                // If depth = 0, gain = 1
                float gain = 1.0f - depth_ * (1.0f - lfo);
                
                outp[i] = inp[i] * gain;
            }
        }
    }
}

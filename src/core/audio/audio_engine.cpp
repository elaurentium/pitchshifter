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

#include <portaudio.h>
#include <iostream>
#include <cstring>

#include "audio_engine.h"
#include "effects/delay.h"
#include "effects/chorus.h"
#include "effects/flanger.h"
#include "effects/reverb.h"
#include "effects/pitchshifter.h"

#define AUDIO_ENGINE_DEBUG 0

namespace PCore {
    AudioEngine::AudioEngine(int sr) : sampleRate(sr) {
        effects.push_back(std::make_unique<PCore::Delay>(sr));
        effects.push_back(std::make_unique<PCore::Chorus>(sr));
        effects.push_back(std::make_unique<PCore::Flanger>(sr));
        effects.push_back(std::make_unique<PCore::Reverb>(sr));
        effects.push_back(std::make_unique<PCore::PitchShifter>(sr));
    }
    
    void AudioEngine::processAudio(std::vector<float> &buffer, int sampleRate) {
        for (auto &effect : effects) {
            effect->process(buffer, sampleRate);
        }
    }

    void AudioEngine::setEffectParameters(size_t effectIndex, const std::string &param, float value) {
        if (effectIndex < effects.size()) {
            effects[effectIndex]->setParameters(param, value);
        }
    }

    int AudioEngine::audioCallBack(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {
        AudioEngine *engine = static_cast<AudioEngine *>(userData);

        if (!engine) return paAbort;

        const float *in = static_cast<const float*>(input);
        float *out = static_cast<float*>(output);

        std::vector<float> buffer(frameCount);
        if (in) {
            std::copy(in, in + frameCount, buffer.begin());
        } else {
            std::fill(buffer.begin(), buffer.end(), 0.0f);
        }

        engine->processAudio(buffer, engine->sampleRate);

        std::copy(buffer.begin(), buffer.end(), out);

        return paContinue;
    } 
}

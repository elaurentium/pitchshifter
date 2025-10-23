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

#if 0
#include <portaudio.h>
#endif

#include <algorithm>
#include <chrono>
#include <cstring>
#include <memory>

#include "audio_engine.h"
#include "effects/delay.h"
#include "effects/chorus.h"
#include "effects/flanger.h"
#include "effects/reverb.h"
#include "effects/pitchshifter.h"

#define AUDIO_ENGINE_DEBUG 0

namespace PCore {
    AudioEngine::AudioEngine(int sr, int block, int inCh, int outCh) : sampleRate_(sr), blockSize_(block), inChans_(inCh), outChans_(outCh), ringIn_( 1024) , ringOut_(1024) {
        engineInBuf_.resize(static_cast<size_t>(blockSize_) * std::max(1, inChans_));
        engineOutBuf_.resize(static_cast<size_t>(blockSize_) * std::max(1, outChans_));
        inPtrs_.resize(std::max(1, inChans_), nullptr);
        outPtrs_.resize(std::max(1, outChans_), nullptr);

        auto g = std::make_unique<AudioGraph>();
        g->addNode(std::make_unique<Delay>(sampleRate_));
        g->addNode(std::make_unique<Chorus>(sampleRate_));
        g->addNode(std::make_unique<Flanger>(sampleRate_));
        g->addNode(std::make_unique<Reverb>(sampleRate_));
        g->addNode(std::make_unique<PitchShifter>(sampleRate_));
        g->prepare(sampleRate_, blockSize_, inChans_, outChans_);
        graph_ = std::move(g);
    }
}

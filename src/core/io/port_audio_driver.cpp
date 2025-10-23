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

#include "port_audio_driver.h"
#include <cstring>

namespace IO
{
    PortAudioDriver::PortAudioDriver() {}
    PortAudioDriver::~PortAudioDriver() { shutdown(); }

    int PortAudioDriver::paCallback(const void *input, void *output, unsigned long frames,
                                    const PaStreamCallbackTimeInfo *, PaStreamCallbackFlags, void *userData) {
        auto *self = static_cast<PortAudioDriver*>(userData);
        const float *in = static_cast<const float*>(input);
        float *out = static_cast<float*>(output);
        if (self->callback_) {
            return self->callback_(in, out, frames);
        }

        if (out) std::memset(out, 0, frames * self->info_.outputChannels * sizeof(float));
        return paContinue;
    }

    bool PortAudioDriver::initialize(const AudioStreamInfo &info, AudioCallback callback, std::string *err) {
        info_ = info;
        callback_ = std::move(callback);
        PaError e = Pa_Initialize();

        if (e != paNoError) {
            if (err) *err = Pa_GetErrorText(e);
            return false;
        }

        // Select default devices; you can add Hydrogen-style selection later
        PaStreamParameters in{}, out{};
        const PaDeviceInfo* inInfo = nullptr; 
        const PaDeviceInfo* outInfo = nullptr;

        if (info_.inputChannels > 0) {
            in.device = Pa_GetDefaultInputDevice();
            if (in.device == paNoDevice) { if (err) *err = "No default input device"; return false; }
            inInfo = Pa_GetDeviceInfo(in.device);
            in.channelCount = info_.inputChannels;
            in.sampleFormat = paFloat32;
            in.suggestedLatency = inInfo->defaultHighInputLatency;
        }

        if (info_.outputChannels > 0) {
            out.device = Pa_GetDefaultOutputDevice();
            if (out.device == paNoDevice) { if (err) *err = "No default output device"; return false; }
            outInfo = Pa_GetDeviceInfo(out.device);
            out.channelCount = info_.outputChannels;
            out.sampleFormat = paFloat32;
            out.suggestedLatency = outInfo->defaultHighOutputLatency;
        }

        e = Pa_OpenStream(&stream_,
                        info_.inputChannels ? &in : nullptr,
                        info_.outputChannels ? &out : nullptr,
                        info_.sampleRate, info_.framesPerBuffer, paClipOff,
                        &PortAudioDriver::paCallback, this);
        if (e != paNoError) {
            if (err) *err = Pa_GetErrorText(e);
            return false;
        }
        initialized_ = true;
        return true;
    }

    bool PortAudioDriver::start(std::string* err) {
        if (!initialized_) { if (err) *err = "Not initialized"; return false; }
        PaError e = Pa_StartStream(stream_);
        if (e != paNoError) { if (err) *err = Pa_GetErrorText(e); return false; }
        return true;
    }

    void PortAudioDriver::stop() {
        if (stream_) {
            Pa_StopStream(stream_);
            Pa_CloseStream(stream_);
            stream_ = nullptr;
        }
    }

    void PortAudioDriver::shutdown() {
        stop();
        Pa_Terminate();
        initialized_ = false;
    }
}
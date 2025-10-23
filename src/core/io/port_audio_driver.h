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

#ifndef PORT_AUDIO_DRIVE
#define PORT_AUDIO_DRIVE

#include "audio_driver.h"
#include <atomic>
#include <portaudio.h>

namespace IO {
    class PortAudioDriver : public AudioDriver {
        public:
            PortAudioDriver();
            ~PortAudioDriver() override;

            const char* name() const override { return "PortAudio"; }
            HostApi hostApi() const override { return HostApi::PortAudioMux; }

            std::vector<DeviceInfo> listDevices() const override;
            int defaultInputDeviceId() const override;
            int defaultOutputDeviceId() const override;

            bool initialize(const StreamConfig& cfg, RtCallback cb, void* userData, std::string* err) override;
            bool start(std::string* err) override;
            void stop() override;
            void shutdown() override;

            double inputLatencySec() const override { return inLatency_; }
            double outputLatencySec() const override { return outLatency_; }
            int sampleRate() const override { return cfg_.sampleRate; }
            int framesPerBlock() const override { return cfg_.framesPerBlock; }

        private:
            static int paCallback(const void* input, void* output, unsigned long frameCount,
                                const PaStreamCallbackTimeInfo* timeInfo,
                                PaStreamCallbackFlags statusFlags, void* userData);

            PaStream* stream_ = nullptr;
            StreamConfig cfg_{};
            RtCallback cb_ = nullptr;
            void* userData_ = nullptr;
            double inLatency_ = 0.0, outLatency_ = 0.0;
            mutable bool paInit_ = false;

            // Working buffers if we need to deinterleave/interleave
            std::vector<float> tmpIn_;
            std::vector<float> tmpOut_;
    };
};


#endif // PORT_AUDIO_DRIVE
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

#ifndef AUDIO_DRIVER_H
#define AUDIO_DRIVER_H

#include <string>
#include <vector>


namespace IO {
    enum class HostApi {
        Unknown, CoreAudio, ALSA, PulseAudio, JACK, WASAPI, ASIO, OSS, PortAudioMux
    };

    struct DeviceInfo {
        int id = -1;
        std::string name;
        HostApi api = HostApi::Unknown;
        int maxInputChannels = 0;
        int maxOutputChannels = 0;
        double defaultSampleRate = 44100.0;
        double defaultLowInLatency = 0.01;
        double defaultLowOutLatency = 0.01;
        double defaultHighInLatency = 0.04;
        double defaultHighOutLatency = 0.04;
    };

    struct StreamConfig {
        int sampleRate = 48000;
        int framesPerBlock = 256;
        int inputChannels = 1;
        int outputChannels = 2;
        int inputDeviceId = -1;   // -1 => default/best
        int outputDeviceId = -1;
        double targetLatencySec = 0.02; // policy hint
        bool interleaved = true; // prefer interleaved; driver may adapt
    };

    enum class CallbackResult { Continue, Complete, Abort };

    // Strict RT callback signature used by driver layer
    using RtCallback = CallbackResult(*)(const float* const* in, float* const* out,
                                     unsigned long frames, void* userData);

    class AudioDriver {
        public:
            virtual ~AudioDriver() = default;
            virtual const char* name() const = 0;
            virtual HostApi hostApi() const = 0;

            // Device enumeration
            virtual std::vector<DeviceInfo> listDevices() const = 0;
            virtual int defaultInputDeviceId() const = 0;
            virtual int defaultOutputDeviceId() const = 0;

            // Lifecycle
            virtual bool initialize(const StreamConfig& cfg, RtCallback cb, void* userData, std::string* err) = 0;
            virtual bool start(std::string* err) = 0;
            virtual void stop() = 0;
            virtual void shutdown() = 0;

            // Metrics
            virtual double inputLatencySec() const = 0;
            virtual double outputLatencySec() const = 0;
            virtual int sampleRate() const = 0;
            virtual int framesPerBlock() const = 0;
    };

}


#endif // AUDIO_DRIVER_H
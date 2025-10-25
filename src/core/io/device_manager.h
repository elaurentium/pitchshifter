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

#include "audio_driver.h"
#include <memory>
#include <string>


namespace IO {
    class DeviceManager {
        public:
            struct Policy {
                std::vector<IO::HostApi> apiPreference;
            	bool preferSameApiForInOut = true;
				Policy() : apiPreference{
					IO::HostApi::CoreAudio,
					IO::HostApi::PulseAudio,
					IO::HostApi::JACK,
					IO::HostApi::ALSA,
					IO::HostApi::WASAPI
				}, preferSameApiForInOut(true) {}
            };

            explicit DeviceManager(std::unique_ptr<IO::AudioDriver> driver, Policy p = Policy()) : driver_(std::move(driver)), policy_(std::move(p)) {};
            bool pickBest(IO::StreamConfig& cfg, std::string* report);
            bool loadConfig(IO::StreamConfig& cfg, const std::string& path, std::string* err);
            bool saveConfig(const IO::StreamConfig& cfg, const std::string& path, std::string* err);

        private:
            std::unique_ptr<IO::AudioDriver> driver_;
            Policy policy_;
    };
}
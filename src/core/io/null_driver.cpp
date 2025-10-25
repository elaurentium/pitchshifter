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

#include "io/audio_driver.h"
#include "io/device_manager.h"
#include <sstream>

namespace IO {
	bool DeviceManager::pickBest(IO::StreamConfig &cfg, std::string *report) {
		std::ostringstream r;

		auto devices = driver_->listDevices();

		if (devices.empty()) {
			if (report) *report = "No devices found";
			return false;
		}
		
		// pick defaults if not set
		if (cfg.inputChannels > 0 && cfg.inputDeviceId < 0) {
			cfg.inputDeviceId = driver_->defaultInputDeviceId();
			r << "Selected input device: " << cfg.inputDeviceId << "\n";
		}
		if (cfg.outputChannels > 0 && cfg.outputDeviceId < 0) {
			cfg.outputDeviceId = driver_->defaultOutputDeviceId();
			r << "Selected output device: " << cfg.outputDeviceId << "\n";
		}

		// clamp channels based on device capabilities
		if (cfg.inputDeviceId >= 0 && cfg.inputDeviceId < (int)devices.size()) {
			cfg.inputChannels = std::max(0, std::min(cfg.inputChannels, devices[cfg.inputDeviceId].maxInputChannels));
		}
		if (cfg.outputDeviceId >= 0 && cfg.outputDeviceId < (int)devices.size()) {
			cfg.outputChannels = std::max(0, std::min(cfg.outputChannels, devices[cfg.outputDeviceId].maxOutputChannels));
		}

		if (report) *report = r.str();
		return true;
	}
}
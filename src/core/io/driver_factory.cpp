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

#include "driver_factory.h"
#include "port_audio_driver.h"

namespace IO {
    DriverFactory& DriverFactory::instance() {
        static DriverFactory inst;
        return inst;
    }

    void DriverFactory::registerDriver(DriverDescriptor d) {
        drivers_.push_back(std::move(d));
    }

    std::unique_ptr<AudioDriver> DriverFactory::createByName(const std::string& name) const {
        for (auto& d : drivers_) {
            if (d.name == name) return d.create();
        }
        return nullptr;
    }

    std::unique_ptr<AudioDriver> DriverFactory::createBestAvailable() const {
        // Very simple: prefer PortAudio; later you can probe availability
        for (auto& d : drivers_) {
            if (d.name == "PortAudio") return d.create();
        }
        if (!drivers_.empty()) return drivers_.front().create();
        return nullptr;
    }

    std::vector<std::string> DriverFactory::listNames() const {
        std::vector<std::string> v;
        v.reserve(drivers_.size());
        for (auto& d : drivers_) v.push_back(d.name);
        return v;
    }

    // Register builtin drivers (call this at startup)
    static bool init = [] {
        DriverFactory::instance().registerDriver({"PortAudio", []{ return std::make_unique<PortAudioDriver>(); }});
        return true;
    }();

}
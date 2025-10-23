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

#include <iostream>
#include <portaudio.h>
#include <vector>
#include <string>

#include "core/audio/audio_engine.h"
#include "io/device_manager.h"
#include "io/port_audio_driver.h"

extern "C" { 
    #include "core/logger.h" 
}

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 2048

int main() {
    // Initialize logger
    Logger* logger = logger_create("pitchshifter.log", true, true, true);
    logger_set_bitmask(Error | Warning | Info | Debug);

    IO::StreamConfig cfg;
    cfg.sampleRate = 48000;
    cfg.framesPerBlock = 512;
    cfg.inputChannels = 1;
    cfg.outputChannels = 2;

    std::unique_ptr<IO::AudioDriver> driver = std::make_unique<IO::PortAudioDriver>();
    IO::DeviceManager dm(std::move(driver));
    std::string rep;
    dm.pickBest(cfg, &rep);
    logger_log(logger, Info, "Main", "DevicePick", rep.c_str());

    // Re-obtain the driver from DeviceManager if it is held.
    std::unique_ptr<IO::AudioDriver> drv = std::make_unique<IO::PortAudioDriver>();
    PCore::AudioEngine engine(cfg.sampleRate, cfg.framesPerBlock, cfg.inputChannels, cfg.outputChannels);

    auto cb = &PCore::AudioEngine::driverCallback;
    std::string err;
    if (!drv->initialize(cfg, cb, &engine, &err)) {
        logger_log(logger, Error, "Main", "Init", err.c_str());
        return -1;
    }
    if (!drv->start(&err)) {
        logger_log(logger, Error, "Main", "Start", err.c_str());
        return -1;
    }

    // Run until ENTER; periodically print metrics from a non-RT thread
    logger_log(logger, Info, "Main", "Run", "Running. Press ENTER to stop.");
    std::atomic<bool> run{true};
    std::thread mon([&]{
        while (run.load()) {
            char buf[256];
            snprintf(buf, sizeof(buf), "CPU=%.2f%% XRuns=%llu LatIn=%.2fms LatOut=%.2fms",
                     engine.cpuLoad() * 100.0,
                     static_cast<unsigned long long>(engine.xruns()),
                     drv->inputLatencySec()*1000.0, drv->outputLatencySec()*1000.0);
            logger_log(logger, Info, "Main", "Metrics", buf);
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    });

    std::cin.get();
    run.store(false);
    mon.join();

    drv->stop();
    drv->shutdown();
    logger_destroy(logger);
    return 0;
}
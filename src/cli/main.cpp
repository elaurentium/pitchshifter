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

extern "C" {
    #include "core/logger.h"
}

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 512

// Helper function to open stream with automatic fallback
static PaError openStreamWithFallback(PaStream** stream, PCore::AudioEngine* engine, Logger* logger) {
    // 1) Try to open default stream first
    PaError err = Pa_OpenDefaultStream(
        stream,
        1, 1,                           // 1 input channel, 1 output channel
        paFloat32,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        &PCore::AudioEngine::audioCallBack,
        engine
    );

    if (err == paNoError) {
        logger_log(logger, Info, "Main", "openStreamWithFallback", 
                   "Default stream opened successfully.");
        return paNoError;
    }

    // 2) If failed, log and try fallback
    char msg[512];
    snprintf(msg, sizeof(msg), 
             "Default stream failed: %s. Trying specific devices...", 
             Pa_GetErrorText(err));
    logger_log(logger, Warning, "Main", "openStreamWithFallback", msg);

    // 3) List all available devices
    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        snprintf(msg, sizeof(msg), "Error counting devices: %s", Pa_GetErrorText(numDevices));
        logger_log(logger, Error, "Main", "openStreamWithFallback", msg);
        return numDevices;
    }

    if (numDevices == 0) {
        logger_log(logger, Error, "Main", "openStreamWithFallback", 
                   "No audio devices found!");
        return paDeviceUnavailable;
    }

    logger_log(logger, Info, "Main", "openStreamWithFallback", 
               "Enumerating PortAudio devices...");

    int inputDev = paNoDevice;
    int outputDev = paNoDevice;

    // List and find valid devices
    for (int i = 0; i < numDevices; ++i) {
        const PaDeviceInfo* devInfo = Pa_GetDeviceInfo(i);
        const PaHostApiInfo* apiInfo = Pa_GetHostApiInfo(devInfo->hostApi);
        
        snprintf(msg, sizeof(msg),
                 "  [%d] %s | API: %s | In: %d | Out: %d",
                 i, 
                 devInfo->name, 
                 apiInfo ? apiInfo->name : "unknown",
                 devInfo->maxInputChannels, 
                 devInfo->maxOutputChannels);
        logger_log(logger, Info, "Main", "openStreamWithFallback", msg);

        // Select first device with input
        if (devInfo->maxInputChannels > 0 && inputDev == paNoDevice) {
            inputDev = i;
        }
        // Select first device with output
        if (devInfo->maxOutputChannels > 0 && outputDev == paNoDevice) {
            outputDev = i;
        }
    }

    // 4) Check if valid devices were found
    if (inputDev == paNoDevice || outputDev == paNoDevice) {
        logger_log(logger, Error, "Main", "openStreamWithFallback",
                   "No suitable input/output devices found.");
        return paDeviceUnavailable;
    }

    // 5) Configure stream parameters
    const PaDeviceInfo* inInfo = Pa_GetDeviceInfo(inputDev);
    const PaDeviceInfo* outInfo = Pa_GetDeviceInfo(outputDev);

    PaStreamParameters inputParams;
    inputParams.device = inputDev;
    inputParams.channelCount = 1;
    inputParams.sampleFormat = paFloat32;
    inputParams.suggestedLatency = inInfo->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    PaStreamParameters outputParams;
    outputParams.device = outputDev;
    outputParams.channelCount = 1;
    outputParams.sampleFormat = paFloat32;
    outputParams.suggestedLatency = outInfo->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;

    snprintf(msg, sizeof(msg),
             "Trying: Input='%s' (API: %s), Output='%s' (API: %s)",
             inInfo->name, 
             Pa_GetHostApiInfo(inInfo->hostApi)->name,
             outInfo->name, 
             Pa_GetHostApiInfo(outInfo->hostApi)->name);
    logger_log(logger, Info, "Main", "openStreamWithFallback", msg);

    // 6) Open stream with specific devices
    err = Pa_OpenStream(
        stream,
        &inputParams,
        &outputParams,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paClipOff,
        &PCore::AudioEngine::audioCallBack,
        engine
    );

    if (err != paNoError) {
        snprintf(msg, sizeof(msg), "Pa_OpenStream failed: %s", Pa_GetErrorText(err));
        logger_log(logger, Error, "Main", "openStreamWithFallback", msg);
        return err;
    }

    logger_log(logger, Info, "Main", "openStreamWithFallback", 
               "Stream opened with specific devices successfully.");
    return paNoError;
}

int main() {
    // Initialize logger
    Logger* logger = logger_create("pitchshifter.log", true, true, true);
    logger_set_bitmask(Error | Warning | Info | Debug);

    logger_log(logger, Info, "Main", "main", "Starting PitchShifter...");

    // Initialize PortAudio
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Error initializing PortAudio: %s", Pa_GetErrorText(err));
        logger_log(logger, Error, "Main", "main", msg);
        logger_destroy(logger);
        return -1;
    }

    logger_log(logger, Info, "Main", "main", "PortAudio initialized successfully.");

    // Create audio engine
    PCore::AudioEngine engine(SAMPLE_RATE);

    // Configure effects (optional)
    engine.setEffectParameters(0, "mix", 0.4f);
    engine.setEffectParameters(1, "depth", 0.2f);
    logger_log(logger, Info, "Main", "main", "Effects configured successfully.");

    // Open stream with automatic fallback
    PaStream* stream = nullptr;
    err = openStreamWithFallback(&stream, &engine, logger);
    if (err != paNoError) {
        char msg[256];
        snprintf(msg, sizeof(msg), 
                 "Failed to open stream after fallback: %s", 
                 Pa_GetErrorText(err));
        logger_log(logger, Error, "Main", "main", msg);
        Pa_Terminate();
        logger_destroy(logger);
        return -1;
    }

    // Start stream
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Error starting stream: %s", Pa_GetErrorText(err));
        logger_log(logger, Error, "Main", "main", msg);
        Pa_CloseStream(stream);
        Pa_Terminate();
        logger_destroy(logger);
        return -1;
    }

    logger_log(logger, Info, "Main", "main", "Stream running! Speak into the microphone.");
    std::cout << "\nProcessing audio in real-time...\n";
    std::cout << "Press ENTER to exit.\n\n";
    std::cin.get();

    // Shutdown properly
    logger_log(logger, Info, "Main", "main", "Shutting down...");
    
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    logger_log(logger, Info, "Main", "main", "Shutdown completed successfully.");
    logger_destroy(logger);

    return 0;
}
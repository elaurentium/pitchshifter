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
#include <cmath>

#include "core/audio/audio_engine.h"
#include "core/logger.h"

#define SAMPLE_RATE 44100
#define BUFFER_SIZE 512

int main() {
    // Initialize logger
    Logger *logger = logger_create("pitchshifter.log", true, true, false);

    // Set logger level
    logger_set_bitmask(Error | Warning | Info | Debug);

    // Initial log
    logger_log(logger, Debug, "Main", "main", "Starting application...");

    PaError err = Pa_Initialize();
    if (err != paNoError) {
        char msg[256];
        snprintf(msg, sizeof(msg), "PortAudio init error: %s", Pa_GetErrorText(err));
        logger_log(logger, Error, "Main", "main", msg);
        logger_destroy(logger);
        return -1;
    }

    PCore::AudioEngine engine(SAMPLE_RATE);

    // Create a test buffer with a simple sine wave
    std::vector<float> audioBuffer(BUFFER_SIZE);
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        audioBuffer[i] = std::sin(2.0f * M_PI * 440.0f * i / SAMPLE_RATE) * 0.5f;
    }
    
    // Configure effects
    engine.setEffectParameters(0, "threshold", 0.6f); // Compressor
    engine.setEffectParameters(1, "mid", 1.2f);       // EQ
    engine.setEffectParameters(6, "time", SAMPLE_RATE * 0.375f); // Delay
    engine.setEffectParameters(7, "mix", 0.3f);       // Reverb
    
    // Process audio
    engine.processAudio(audioBuffer, SAMPLE_RATE);

    PaStream *stream;
    err = Pa_OpenDefaultStream(
        &stream,
        1, 1,
        paFloat32,
        SAMPLE_RATE,
        BUFFER_SIZE,
        &PCore::AudioEngine::audioCallBack,
        &engine
    );

    if (err != paNoError) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Failed to open stream: %s", Pa_GetErrorText(err));
        logger_log(logger, Error, "Main", "main", msg);
        logger_destroy(logger);
        return -1;
    } 

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Erro to initialize stream: %s", Pa_GetErrorText(err));
        logger_log(logger, Error, "Main", "main", msg);
        Pa_CloseStream(stream);
        Pa_Terminate();
        logger_destroy(logger);
        return -1;
    }

    logger_log(logger, Info, "Main", "main", "Begining simulation...");

    return 0;
}
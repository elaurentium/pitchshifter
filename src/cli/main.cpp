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

#define SAMPLE_RATE 44100
#define BUFFER_SIZE 512


int main() {
    //Pa_Initialize();

    //PaStream *stream;

    //Pa_OpenDefaultStream(&stream, 2, 2, paFloat32, 4800, 256, &PCore::AudioEngine::audioCallBack, nullptr);

    //Pa_StartStream(stream);
    //getchar(); // Keep running until key pressed
    //Pa_StopStream(stream);
    //Pa_CloseStream(stream);
    //Pa_Terminate();

    PCore::AudioEngine vocal300(SAMPLE_RATE);

    // Create a test buffer with a simple sine wave
    std::vector<float> audioBuffer(BUFFER_SIZE);
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        audioBuffer[i] = std::sin(2.0f * M_PI * 440.0f * i / SAMPLE_RATE) * 0.5f;
    }
    
    // Configure effects
    vocal300.setEffectParameters(0, "threshold", 0.6f); // Compressor
    vocal300.setEffectParameters(1, "mid", 1.2f);       // EQ
    vocal300.setEffectParameters(6, "time", SAMPLE_RATE * 0.375f); // Delay
    vocal300.setEffectParameters(7, "mix", 0.3f);       // Reverb
    
    // Process audio
    vocal300.processAudio(audioBuffer);
    
    std::cout << "DigiTech Vocal 300 simulation processed " 
              << BUFFER_SIZE << " samples" << std::endl;

    return 0;
}
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

#ifndef DELAY_H
#define DELAY_H

#include <vector>

#include "../audio_node.h"


namespace PCore {
    class Delay : public AudioNode {
        private:
            std::vector<float> dBuffer_;
            int sampleRate_ = 44100;
            int dTimeMs_ = 250;  //ms
            int writePos_ = 0;
            float feedback_ = 0.35f;
            float mix_ = 0.3f;

            
        public:
            explicit Delay(int sampleRate);
            void prepare(int sr, int block, int inCh, int outCh) override;
            void process(const float* const* in, float* const* out, unsigned long frames) override;
            void setParameters(const std::string& param, float value);
    };
}

#endif // DELAY_H
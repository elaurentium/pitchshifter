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

#include "audio_node.h"
#include <memory>
#include <vector>

namespace PCore {
    class AudioGraph {
        public:
            void addNode(std::unique_ptr<AudioNode> node);
            void clear();
            void prepare(int sampleRate, int maxBlock, int inChans, int outChans);
            void process(const float* const* in, float* const* out, unsigned long frames);

        private:
            std::vector<std::unique_ptr<AudioNode>> nodes_;
            // Preallocated intermediate buffers
            std::vector<std::vector<float>> interleaves_;
            std::vector<float*> interPtrsIn_;
            std::vector<float*> interPtrsOut_;

    };
}
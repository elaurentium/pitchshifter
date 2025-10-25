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

#include "audio_graph.h"

namespace PCore {
	void AudioGraph::addNode(std::unique_ptr<AudioNode> node) {
		nodes_.push_back(std::move(node));
	}

	void AudioGraph::clear() {
		nodes_.clear();
		interleaves_.clear();
		interPtrsIn_.clear();
		interPtrsOut_.clear();
	}

	void AudioGraph::prepare(int sampleRate, int maxBlock, int inCh, int onCh) {
		for (auto &n : nodes_) {
			n->prepare(sampleRate, maxBlock, inCh, onCh);
		}

		const int numCh = std::max(inCh, onCh);
		interleaves_.assign(static_cast<size_t>(numCh), std::vector<float>(static_cast<size_t>(maxBlock), 0.0f));

		interPtrsIn_.resize(static_cast<size_t>(numCh));
		interPtrsOut_.resize(static_cast<size_t>(numCh));
		for (int c = 0; c < numCh; ++c) {
			interPtrsIn_[static_cast<size_t>(c)]  = interleaves_[static_cast<size_t>(c)].data();
			interPtrsOut_[static_cast<size_t>(c)] = interleaves_[static_cast<size_t>(c)].data();
		}
	}

	void AudioGraph::process(const float* const* in, float* const* out, unsigned long frames) {
		const int numInter = static_cast<int>(interleaves_.size());

		if (in && numInter > 0) {
			for (int c = 0; c < numInter; ++c) {
				const float* src = (in[c] != nullptr) ? in[c] : nullptr;
				float* dst = interleaves_[static_cast<size_t>(c)].data();
				if (src) {
					std::memcpy(dst, src, static_cast<size_t>(frames) * sizeof(float));
				} else {
					std::memset(dst, 0, static_cast<size_t>(frames) * sizeof(float));
				}
			}
		} else if (numInter > 0) {
			for (int c = 0; c < numInter; ++c) {
				std::memset(interleaves_[static_cast<size_t>(c)].data(), 0, static_cast<size_t>(frames) * sizeof(float));
			}
		}

		const float* const* curIn = const_cast<const float* const*>(interPtrsIn_.data());
		float* const* curOut = interPtrsOut_.data();

		for (auto& n : nodes_) {
			n->process(curIn, curOut, frames);
			curIn = const_cast<const float* const*>(interPtrsIn_.data());
			curOut = interPtrsOut_.data();
		}

		if (out) {
			for (int c = 0; c < numInter; ++c) {
				float* dst = out[c];
				const float* src = interleaves_[static_cast<size_t>(c)].data();
				if (dst) {
					std::memcpy(dst, src, static_cast<size_t>(frames) * sizeof(float));
				}
			}
		}
	}

}
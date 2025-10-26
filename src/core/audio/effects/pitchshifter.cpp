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

#include <cmath>
#include "pitchshifter.h"
#include <algorithm>

// TODO: learn how to performated pitchshifter works cuz sounds weird asf
namespace PCore {
    PitchShifter::PitchShifter(int sampleRate) : sampleRate_(sampleRate) {}

    void PitchShifter::prepare(int sr, int block, int inCh, int outCh) {
		sampleRate_ = sr;

		// Choose ~50ms buffer for time-domain pitch shifting (tradeoff quality/latency)
		bufferSize_ = std::max(128, static_cast<int>(std::lround(sampleRate_ * 0.05))); // ~50ms
		buffer1_.assign(bufferSize_, 0.0f);
		buffer2_.assign(bufferSize_, 0.0f);
		window_.resize(bufferSize_);
		rebuildWindow();

		writePos_  = 0;
		readPos1_  = 0.0f;
		readPos2_  = 0.5f * static_cast<float>(bufferSize_); // 50% offset for smoother crossfade
		crossfade_ = 0.0f;
	}

	inline float PitchShifter::hannAt(int idx) const {
		// Hann window: 0.5 * (1 - cos(2π n / N))
		// Guard against empty window (shouldn't happen after prepare)
		if (window_.empty()) return 1.0f;
		int N = static_cast<int>(window_.size());
		int i = idx % N;
		return window_[i];
	}

	void PitchShifter::rebuildWindow() {
		if (bufferSize_ <= 0) return;
		window_.resize(bufferSize_);
		for (int i = 0; i < bufferSize_; ++i) {
			window_[i] = 0.5f * (1.0f - std::cos(2.0f * static_cast<float>(M_PI) * (static_cast<float>(i) / static_cast<float>(bufferSize_))));
		}
	}

	void PitchShifter::process(const float* const* in, float* const* out, unsigned long frames) {
		if (!out || !out[0]) return;

		const float* x = (in && in[0]) ? in[0] : nullptr;
		float* y0 = out[0];

		if (!x) {
			std::fill_n(y0, frames, 0.0f);
			// duplicate to additional channels if any
			return;
		}

		// Adaptive crossfade speed based on current buffer size
		const float crossfadeSpeed = (bufferSize_ > 0) ? (2.0f / static_cast<float>(bufferSize_)) : 0.0f;

		const int N = bufferSize_;
		if (N <= 1) {
			// Fallback: passthrough
			std::copy(x, x + frames, y0);
			return;
		}

		for (unsigned long i = 0; i < frames; ++i) {
			const float inS = x[i];

			// Write input into both circular buffers
			buffer1_[writePos_] = inS;
			buffer2_[writePos_] = inS;

			// Read positions and fractional parts
			int   r1i = static_cast<int>(readPos1_);
			int   r2i = static_cast<int>(readPos2_);
			float f1  = readPos1_ - static_cast<float>(r1i);
			float f2  = readPos2_ - static_cast<float>(r2i);

			// Wrap and next indices
			int r1n = (r1i + 1) % N;
			int r2n = (r2i + 1) % N;

			// Linear interpolation
			float s1 = buffer1_[r1i] * (1.0f - f1) + buffer1_[r1n] * f1;
			float s2 = buffer2_[r2i] * (1.0f - f2) + buffer2_[r2n] * f2;

			// Apply Hann window to reduce artifacts (indexing by read head; modulo N)
			s1 *= hannAt(r1i);
			s2 *= hannAt(r2i);

			// Equal-power crossfade
			float cf   = std::clamp(crossfade_ * 0.5f, 0.0f, 1.0f);   // map [0..2) → [0..1]
			float g1   = std::cos(cf * static_cast<float>(M_PI) * 0.5f);
			float g2   = std::sin(cf * static_cast<float>(M_PI) * 0.5f);
			float wet  = s1 * g1 + s2 * g2;

			// Mix dry/wet
			y0[i] = (1.0f - mix_) * inS + mix_ * wet;

			// Advance read heads by pitch ratio
			readPos1_ += pitchRatio_;
			readPos2_ += pitchRatio_;
			crossfade_ += crossfadeSpeed;

			// Wrap read heads and crossfade
			if (readPos1_ >= N) readPos1_ -= static_cast<float>(N);
			if (readPos2_ >= N) readPos2_ -= static_cast<float>(N);
			if (crossfade_ >= 2.0f) crossfade_ -= 2.0f;

			// Advance write head
			writePos_ = (writePos_ + 1) % N;
		}

		// Duplicate to additional output channels if needed
		// If your graph uses non-interleaved channels, copy y0 into out[1..] here.
	}

	void PitchShifter::setParameters(const std::string& param, float value) {
		if (param == "pitch") {
			// Semitones → ratio (12-TET)
			pitchRatio_ = std::pow(2.0f, value / 12.0f);
			// “Formant” placeholder kept for API compatibility (no-op in this algorithm)
			formantRatio_ = std::pow(2.0f, -value / 24.0f); // not applied in this simple TD shifter
		} else if (param == "formant") {
			// Placeholder: reserved for more advanced algorithms (phase vocoder/PSOLA)
			formantRatio_ = std::pow(2.0f, value / 12.0f);
		} else if (param == "mix") {
			mix_ = std::clamp(value, 0.0f, 1.0f);
		} else if (param == "buffer_ms") {
			// Allow changing internal buffer size (rebuilds window)
			float ms = std::clamp(value, 10.0f, 100.0f); // 10..100 ms typical
			bufferSize_ = std::max(128, static_cast<int>(std::lround(sampleRate_ * (ms * 0.001f))));
			buffer1_.assign(bufferSize_, 0.0f);
			buffer2_.assign(bufferSize_, 0.0f);
			rebuildWindow();
			writePos_  = 0;
			readPos1_  = 0.0f;
			readPos2_  = 0.5f * static_cast<float>(bufferSize_);
			crossfade_ = 0.0f;
		}
	}

	int PitchShifter::latencySamples() const {
		// Rough estimate: half the buffer because we offset read head by 50%
		return bufferSize_ / 2;
	}
}
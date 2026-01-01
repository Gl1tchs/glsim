#include "core/timer.h"

namespace gl {

Timer::Timer() { reset(); }

void Timer::reset() {
	_start_time = Clock::now();
	_last_frame_time = _start_time;
	_delta_time = 0.0f;
}

void Timer::tick() {
	const auto current_time = Clock::now();

	// Calculate delta
	std::chrono::duration<float> delta = current_time - _last_frame_time;
	_delta_time = delta.count();

	// Update timestamp
	_last_frame_time = current_time;
}

float Timer::get_delta_time() const { return _delta_time; }

float Timer::get_delta_milliseconds() const { return _delta_time * 1000.0f; }

double Timer::get_total_time() const {
	const auto current_time = Clock::now();
	std::chrono::duration<double> total = current_time - _start_time;
	return total.count();
}

} // namespace gl

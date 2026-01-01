#pragma once

namespace gl {

class Timer {
public:
	using Clock = std::chrono::steady_clock;

	Timer();

	// Resets the timer to zero.
	void reset();

	// Calculates the time difference between this call and the previous tick.
	void tick();

	/**
	 * Returns the time in seconds between the last two tick() calls.
	 * Does not modify state.
	 */
	float get_delta_time() const;

	// Returns the time in milliseconds between the last two tick() calls.
	float get_delta_milliseconds() const;

	// Returns the total time in seconds since the timer was created or reset.
	double get_total_time() const;

private:
	std::chrono::time_point<Clock> _start_time;
	std::chrono::time_point<Clock> _last_frame_time;
	float _delta_time = 0.0f;
};

} // namespace gl

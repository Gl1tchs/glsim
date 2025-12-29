#include "core/random.h"

namespace gl {

float random_float(float min, float max) {
	static std::ptrdiff_t seed = 0;
	static std::mt19937 generator(std::random_device{}());

	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(generator);
}

int random_int(int min, int max) {
	static std::mt19937 generator(std::random_device{}());
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(generator);
}

} //namespace gl

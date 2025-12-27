#include "core/uid.h"

namespace gl {

static std::random_device random_device;

static thread_local std::mt19937_64 engine(random_device());
static thread_local std::uniform_int_distribution<uint32_t> uniform_distribution;

UID::UID() : value(uniform_distribution(engine)) {}

UID::UID(const uint32_t& uuid) : value(uuid) {}

UID::UID(uint32_t&& uuid) : value(std::move(uuid)) {}

UID& UID::operator=(const UID& other) {
	value = (uint32_t)other;
	return *this;
}

UID& UID::operator=(UID&& other) {
	value = (uint32_t)other;
	return *this;
}

UID& UID::operator=(const uint32_t& other) {
	value = other;
	return *this;
}

UID& UID::operator=(uint32_t&& other) {
	value = other;
	return *this;
}

} //namespace gl

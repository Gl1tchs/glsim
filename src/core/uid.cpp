#include "core/uid.h"

namespace gl {

static uint64_t s_counter = 0;

UID::UID() : value(++s_counter) {}

UID::UID(const uint64_t& uid) : value(uid) {}

UID::UID(uint64_t&& uid) : value(std::move(uid)) {}

UID& UID::operator=(const UID& other) {
	value = (uint64_t)other;
	return *this;
}

UID& UID::operator=(UID&& other) {
	value = (uint64_t)other;
	return *this;
}

UID& UID::operator=(const uint64_t& other) {
	value = other;
	return *this;
}

UID& UID::operator=(uint64_t&& other) {
	value = other;
	return *this;
}

} //namespace gl

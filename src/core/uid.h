#pragma once

namespace gl {

struct UID {
	uint64_t value;

	UID();
	UID(const uint64_t& uid);
	UID(uint64_t&& uuid);
	UID(const UID&) = default;

	UID& operator=(const UID& other);
	UID& operator=(UID&& other);

	UID& operator=(const uint64_t& other);
	UID& operator=(uint64_t&& other);

	bool is_valid() const { return value != 0; }

	operator uint64_t() const { return value; }
};

inline const UID INVALID_UID = 0;

} //namespace gl

namespace std {
template <> struct hash<gl::UID> {
	size_t operator()(const gl::UID& uuid) const { return (uint64_t)uuid; }
};
} //namespace std

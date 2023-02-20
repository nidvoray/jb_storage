#ifndef STORAGE_COMMON_H
#define STORAGE_COMMON_H

#include <string>
#include <variant>
#include <vector>

namespace jb_storage
{

	using Blob = std::vector<uint8_t>;

	using Value = std::variant<std::monostate, uint32_t, uint64_t, float, double, std::string, Blob>;

}

#endif

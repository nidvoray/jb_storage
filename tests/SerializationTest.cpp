#include "Serialization.h"

#include <gtest/gtest.h>

#include <sstream>

using namespace jb_storage;

TEST(SerializationTest, AllTypes)
{
	const Value uint64{ uint64_t{ 0xFF000000000000FF } };
	const Value uint32{ uint32_t{ 0xFF0000FF } };
	const Value flt{ 1.f };
	const Value dbl{ 1. };
	const Value str{ "42" };
	const Value blob{ Blob{ 4, 2 } };
	const Value none{ };

	std::stringstream stream(std::ios_base::in | std::ios_base::out | std::ios_base::binary);

	utility::Serialize(uint64, stream);
	utility::Serialize(uint32, stream);
	utility::Serialize(flt, stream);
	utility::Serialize(dbl, stream);
	utility::Serialize(str, stream);
	utility::Serialize(blob, stream);
	utility::Serialize(none, stream);

	stream.seekg(0, std::ios::beg);

	ASSERT_TRUE(utility::Deserialize<Value>(stream) == uint64);
	ASSERT_TRUE(utility::Deserialize<Value>(stream) == uint32);
	ASSERT_TRUE(utility::Deserialize<Value>(stream) == flt);
	ASSERT_TRUE(utility::Deserialize<Value>(stream) == dbl);
	ASSERT_TRUE(utility::Deserialize<Value>(stream) == str);
	ASSERT_TRUE(utility::Deserialize<Value>(stream) == blob);
	ASSERT_TRUE(utility::Deserialize<Value>(stream) == none);
}

#include "Storage.h"
#include "TestSet.h"

#include <gtest/gtest.h>

#include <sstream>

using namespace jb_storage;

TEST(SaveLoadTest, SaveLoad)
{
	const Volume src;
	const auto test_set{ GenerateTestSet("", 5, 4) };

	for (const auto& entity : test_set)
		ASSERT_TRUE(src.SetOrInsert(entity.Path, entity.Value_));

	std::stringstream stream(std::ios_base::in | std::ios_base::out | std::ios_base::binary);

	ASSERT_TRUE(src.Save(stream));

	const Volume dst;

	stream.seekg(0, std::ios::beg);
	ASSERT_TRUE(dst.Load(stream));

	for (const auto& entity : test_set)
	{
		const auto val{ dst.Get(entity.Path) };
		ASSERT_TRUE(val);
		ASSERT_TRUE(*val == entity.Value_);
	}
}

TEST(SaveLoadTest, DontSaveLoadIfMounted)
{
	const Volume src;
	const Storage storage;

	std::stringstream stream(std::ios_base::in | std::ios_base::out | std::ios_base::binary);

	{
		const auto token{ storage.Mount("/", src, "/") };
		ASSERT_TRUE(token);

		ASSERT_TRUE(storage.SetOrInsert("/foo/bar", uint64_t{ 42 }));

		ASSERT_FALSE(src.Save(stream));
	}

	ASSERT_TRUE(src.Save(stream));

	const Volume dst;
	stream.seekg(0, std::ios::beg);

	{
		const auto token{ storage.Mount("/", dst, "/") };
		ASSERT_TRUE(token);

		ASSERT_FALSE(dst.Load(stream));
	}

	ASSERT_TRUE(dst.Load(stream));

	const auto bar{ dst.Get("/foo/bar") };
	ASSERT_NO_THROW(ASSERT_TRUE(bar && std::get<uint64_t>(*bar) == 42));
}

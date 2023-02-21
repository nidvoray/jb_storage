#include "Volume.h"

#include <gtest/gtest.h>

using namespace jb_storage;

TEST(VolumeTest, SetRoot)
{
	const Volume volume;

	ASSERT_TRUE(volume.SetOrInsert("/", 42u));

	const auto value{ volume.Get("/") };
	ASSERT_TRUE(value && std::holds_alternative<uint32_t>(*value) && std::get<uint32_t>(*value) == 42u);
}

TEST(VolumeTest, DeleteRoot)
{
	const Volume volume;
	ASSERT_FALSE(volume.Delete("/"));
}

TEST(VolumeTest, SetAndResetNonRoot)
{
	const Volume volume;

	ASSERT_TRUE(volume.SetOrInsert("/foo/bar", 42u));

	{
		const auto bar{ volume.Get("/foo/bar") };
		const auto foo{ volume.Get("/foo") };

		ASSERT_NO_THROW(ASSERT_TRUE(bar && std::get<uint32_t>(*bar) == 42u));
		ASSERT_TRUE(foo && std::holds_alternative<std::monostate>(*foo));
	}

	ASSERT_TRUE(volume.SetOrInsert("/foo/bar/baz", 42.));
	ASSERT_TRUE(volume.SetOrInsert("/foo/bar", "42"));
	ASSERT_TRUE(volume.SetOrInsert("/foo", 42u));

	{
		const auto foo{ volume.Get("/foo") };
		const auto bar{ volume.Get("/foo/bar") };
		const auto baz{ volume.Get("/foo/bar/baz") };

		ASSERT_NO_THROW(ASSERT_TRUE(foo && std::get<uint32_t>(*foo) == 42u));
		ASSERT_NO_THROW(ASSERT_TRUE(bar && std::get<std::string>(*bar) == "42"));
		ASSERT_NO_THROW(ASSERT_TRUE(baz && std::get<double>(*baz) == 42.));
	}
}

TEST(VolumeTest, DeleteNonRoot)
{
	const Volume volume;

	ASSERT_TRUE(volume.SetOrInsert("/foo/bar/baz", 42u));
	ASSERT_TRUE(volume.SetOrInsert("/foo/bar/qux", "42"));
	ASSERT_TRUE(volume.SetOrInsert("/foo", 42.));

	ASSERT_TRUE(volume.Delete("/foo/bar/baz"));
	ASSERT_FALSE(volume.Get("/foo/bar/baz"));

	{
		const auto qux{ volume.Get("/foo/bar/qux") };
		ASSERT_NO_THROW(ASSERT_TRUE(qux && std::get<std::string>(*qux) == "42"));
	}

	ASSERT_FALSE(volume.Delete("/foo/bar/baz"));
	ASSERT_TRUE(volume.Delete("/foo/bar"));

	ASSERT_FALSE(volume.Get("/foo/bar/baz"));
	ASSERT_FALSE(volume.Get("/foo/bar"));

	{
		const auto foo{ volume.Get("/foo") };
		ASSERT_NO_THROW(ASSERT_TRUE(foo && std::get<double>(*foo) == 42.));
	}
}

#include "Storage.h"

#include <gtest/gtest.h>

using namespace jb_storage;

TEST(StorageTest, MountUnmount)
{
	const Volume volume;
	const Storage storage;

	ASSERT_TRUE(volume.SetOrInsert("/foo/bar", 42u));

	{
		const auto token{ storage.Mount("/vol", volume, "/") };

		ASSERT_TRUE(token);

		const auto bar{ storage.Get("/vol/foo/bar") };
		ASSERT_NO_THROW(ASSERT_TRUE(bar && std::get<uint32_t>(*bar) == 42u));
	}

	ASSERT_TRUE(volume.Get("/foo/bar"));
	ASSERT_FALSE(storage.Get("/vol/foo/bar"));
}

TEST(StorageTest, MountAbsentPath)
{
	const Storage storage;
	const Volume volume;

	ASSERT_FALSE(storage.Mount("/vol", volume, "/foo"));
}

TEST(StorageTest, MountSameNodeToDifferentPoints)
{
	const Volume volume;
	const Storage storage;

	ASSERT_TRUE(volume.SetOrInsert("/foo/bar", 42u));

	{
		const auto token1{ storage.Mount("/mp1", volume, "/") };
		const auto token2{ storage.Mount("/mp2", volume, "/") };

		ASSERT_TRUE(token1 && token2);

		const auto mp1bar{ storage.Get("/mp1/foo/bar") };
		ASSERT_NO_THROW(ASSERT_TRUE(mp1bar && std::get<uint32_t>(*mp1bar) == 42u));

		auto mp2bar{ storage.Get("/mp2/foo/bar") };
		ASSERT_NO_THROW(ASSERT_TRUE(mp1bar && std::get<uint32_t>(*mp1bar) == 42u));

		ASSERT_TRUE(storage.SetOrInsert("/mp1/foo/bar", "newval"));

		mp2bar = storage.Get("/mp2/foo/bar");
		ASSERT_NO_THROW(ASSERT_TRUE(mp2bar && std::get<std::string>(*mp2bar) == "newval"));
	}

	const auto bar{ volume.Get("/foo/bar") };
	ASSERT_NO_THROW(ASSERT_TRUE(bar && std::get<std::string>(*bar) == "newval"));

	ASSERT_FALSE(storage.Get("/mp1/foo/bar") || storage.Get("/mp2/foo/bar"));
}

TEST(StorageTest, Priority)
{
	const Volume volume;
	const Storage storage;

	ASSERT_TRUE(volume.SetOrInsert("/foo/bar", "lowpriority") && volume.SetOrInsert("/foo/bar/bar", "highpriority"));

	{
		const auto token{ storage.Mount("/vol", volume, "/foo") };
		ASSERT_TRUE(token);

		{
			const auto token{ storage.Mount("/vol", volume, "/foo/bar") };
			ASSERT_TRUE(token);

			const auto bar{ storage.Get("/vol/bar") };
			ASSERT_NO_THROW(ASSERT_TRUE(bar && std::get<std::string>(*bar) == "highpriority"));

			ASSERT_TRUE(storage.SetOrInsert("/vol/bar", "highnewval"));
		}

		const auto bar{ storage.Get("/vol/bar") };
		ASSERT_NO_THROW(ASSERT_TRUE(bar && std::get<std::string>(*bar) == "lowpriority"));

		ASSERT_TRUE(storage.SetOrInsert("/vol/bar", "lownewval"));
	}

	const auto bar{ volume.Get("/foo/bar") };
	ASSERT_NO_THROW(ASSERT_TRUE(bar && std::get<std::string>(*bar) == "lownewval"));

	const auto barbar{ volume.Get("/foo/bar/bar") };
	ASSERT_NO_THROW(ASSERT_TRUE(barbar && std::get<std::string>(*barbar) == "highnewval"));
}

TEST(StorageTest, Delete)
{
	const Volume volume;
	const Storage storage;

	ASSERT_TRUE(volume.SetOrInsert("/foo/bar/baz", 42u) && volume.SetOrInsert("/foo/bar/qux", "42"));

	{
		const auto token{ storage.Mount("/vol", volume, "/foo/bar") };
		ASSERT_TRUE(token);

		ASSERT_TRUE(storage.Delete("/vol/baz") && storage.Delete("/vol/qux"));
		ASSERT_FALSE(storage.Delete("/vol/none"));
	}

	ASSERT_TRUE(volume.Get("/foo/bar"));
	ASSERT_FALSE(volume.Get("/foo/bar/baz") || volume.Get("/foo/bar/qux"));
}

TEST(StorageTest, DeleteVirtualNodeWontDeleteRealOnes)
{
	const Volume volume;
	const Storage storage;

	ASSERT_TRUE(volume.SetOrInsert("/foo/bar/baz", 42u) && volume.SetOrInsert("/foo/bar/qux", "42"));

	{
		const auto token{ storage.Mount("/vol", volume, "/foo/bar") };
		ASSERT_TRUE(token);

		ASSERT_TRUE(storage.Delete("/vol"));
	}

	ASSERT_TRUE(volume.Get("/foo/bar/baz") && volume.Get("/foo/bar/qux"));
}

TEST(StorageTest, NoDataInVirtualNode)
{
	const Volume volume;
	const Storage storage;

	ASSERT_TRUE(volume.SetOrInsert("/foo/bar", 42u));

	{
		const auto token{ storage.Mount("/foo/vol", volume, "/foo/bar") };
		ASSERT_TRUE(token);

		ASSERT_FALSE(storage.Get("/foo"));
		ASSERT_FALSE(storage.SetOrInsert("/foo", 42u));
	}
}

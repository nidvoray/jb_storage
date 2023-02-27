#include "Storage.h"
#include "TestSet.h"

#include <gtest/gtest.h>
#include <random>
#include <thread>

using namespace jb_storage;

namespace
{

	std::random_device random_device;

}

TEST(StabilityTest, SetAsync)
{
	const Volume volume;
	const auto test_set{ GenerateTestSet("", 4, 4) };

	std::vector<std::thread> threads;
	for (const auto& entity : test_set)
		threads.emplace_back([&entity, &volume]()
		{ ASSERT_TRUE(volume.SetOrInsert(entity.Path, entity.Value_));});

	for (auto& thread : threads)
		thread.join();

	for (const auto& entity : test_set)
	{
		const auto val{ volume.Get(entity.Path) };
		ASSERT_TRUE(val);
		ASSERT_TRUE(*val == entity.Value_);
	}
}

TEST(StabilityTest, DeleteAsync)
{
	const Volume volume;
	const auto test_set{ GenerateTestSet("", 4, 4) };

	for (const auto& entity : test_set)
		ASSERT_TRUE(volume.SetOrInsert(entity.Path, entity.Value_));

	std::vector<std::thread> threads;
	for (const auto& entity : test_set)
		threads.emplace_back([&entity, &volume]()
		{ volume.Delete(entity.Path); }); // no need to check retval here due to random order of deleting

	for (auto& thread : threads)
		thread.join();

	for (const auto& entity : test_set)
		ASSERT_FALSE(volume.Get(entity.Path));
}

TEST(StabilityTest, SetAsyncWhileBusyLoopGetAsync)
{
	const Volume volume;
	const auto test_set{ GenerateTestSet("", 3, 4) };

	std::vector<std::thread> setters;
	std::vector<std::thread> getters;

	std::vector<size_t> indices;
	std::vector<std::unique_ptr<std::atomic<bool>>> ready;

	for (size_t i{ 0 }, size{ test_set.size() }; i < size; ++i)
	{
		indices.push_back(i);
		ready.push_back(std::make_unique<std::atomic<bool>>(false));
	}

	std::shuffle(indices.begin(), indices.end(), random_device);

	for (size_t i{ 0 }, size{ test_set.size() }; i < size; ++i)
	{
		setters.emplace_back([&, i]()
		{
			const auto& entity{ test_set[i] };
			ASSERT_TRUE(volume.SetOrInsert(entity.Path, entity.Value_));
			*ready[i] = true;
		});

		getters.emplace_back([&, i]()
		{
			const auto index{ indices[i] };
			const auto& entity{ test_set[index] };

			while (!*ready[index])
			{
				volume.Get(entity.Path);
				std::this_thread::yield();
			}

			const auto val{ volume.Get(entity.Path) };
			ASSERT_TRUE(val && *val == entity.Value_);
		});
	}

	for (auto& thread : setters)
		thread.join();

	for (auto& thread : getters)
		thread.join();
}

TEST(StabilityTest, MountedVolumesAsyncSet)
{
	const Storage storage;
	auto mount_points{ GenerateTestSet("", 3, 2) };

	struct MountedVolume
	{
		Volume		Volume_;
		TestSet		TestSet_;
		std::string	MountPoint;
	};

	std::vector<MountedVolume> mounted_volumes;
	for (auto& mp : mount_points)
		mounted_volumes.push_back(MountedVolume{ Volume{ }, GenerateTestSet("", 4, 2), std::move(mp.Path) });

	std::vector<std::thread> threads;

	for (const auto& mounted_volume : mounted_volumes)
		threads.emplace_back([&mounted_volume, &storage]()
		{
			const auto token{ storage.Mount(mounted_volume.MountPoint, mounted_volume.Volume_, "/") };
			ASSERT_TRUE(token);

			std::vector<std::thread> threads;
			for (const auto& entity : mounted_volume.TestSet_)
				threads.emplace_back([&entity, &mounted_volume, &storage]()
				{ ASSERT_TRUE(storage.SetOrInsert(mounted_volume.MountPoint + entity.Path, entity.Value_));});

			for (auto& thread : threads)
				thread.join();
		});

	for (auto& thread : threads)
		thread.join();

	for (const auto& mounted_volume : mounted_volumes)
		for (const auto& entity : mounted_volume.TestSet_)
		{
			const auto val{ mounted_volume.Volume_.Get(entity.Path) };
			ASSERT_TRUE(val);
			ASSERT_TRUE(*val == entity.Value_);
		}
}

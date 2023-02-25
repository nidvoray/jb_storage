#include "Storage.h"

#include <gtest/gtest.h>
#include <random>
#include <thread>

using namespace jb_storage;

namespace
{

	std::random_device random_device;

	struct TestEntity
	{
		std::string	Path;
		Value		Value_;
	};

	using TestSet = std::vector<TestEntity>;

	// note that total size of set (and number of threads as well) is sum([node_power**i for i in range (1,level + 1)])
	// please choose arguments so as not to exceed your OS limitation
	TestSet GenerateTestSet(const std::string& uplevel_path, size_t node_power, size_t level)
	{
		if (!level || !node_power)
			return TestSet{ };

		const auto level_generator{ [node_power](TestSet& test_set, const std::string& uplevel_path, size_t level, const auto& self) -> void
		{
			std::set<unsigned> subkeys;
			while (subkeys.size() != node_power)
				subkeys.insert(random_device());

			for (const auto subkey : subkeys)
			{
				const auto type_rnd{ std::uniform_int_distribution<>{ 0, 5 }(random_device) };
				const auto value_rnd{ random_device() };

				Value value{
						type_rnd == 0 ? Value{ static_cast<uint32_t>(value_rnd) } :
						type_rnd == 1 ? Value{ static_cast<uint64_t>(value_rnd) } :
						type_rnd == 2 ? Value{ static_cast<double>(value_rnd) } :
						type_rnd == 3 ? Value{ static_cast<float>(value_rnd) } :
						type_rnd == 4 ? Value{ std::to_string(value_rnd) } :
						Value{ [value_rnd]()
						{
							const auto data_begin{ reinterpret_cast<Blob::const_pointer>(&value_rnd) };
							Blob value;

							// endianess doesn't matter here
							std::copy(data_begin, std::next(data_begin, sizeof(value_rnd)), std::back_inserter(value));
							return value;
						}() }
				};

				const auto path{ uplevel_path + '/' + std::to_string(subkey) };
				test_set.push_back(TestEntity{ path, std::move(value) });
				if (level - 1)
					self(test_set, path, level - 1, self);
			}
		} };

		TestSet ret;
		level_generator(ret, uplevel_path, level, level_generator);

		std::shuffle(ret.begin(), ret.end(), random_device);

		return ret;
	}

}

TEST(StabilityTest, SetAsync)
{
	Volume volume;
	const auto test_set{ GenerateTestSet("", 5, 6) };

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
	Volume volume;
	const auto test_set{ GenerateTestSet("", 5, 6) };

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
	Volume volume;
	const auto test_set{ GenerateTestSet("", 4, 5) };

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
	Storage storage;
	auto mount_points{ GenerateTestSet("", 3, 2) };

	struct MountedVolume
	{
		Volume		Volume_;
		TestSet		TestSet_;
		std::string	MountPoint;
	};

	std::vector<MountedVolume> mounted_volumes;
	for (auto& mp : mount_points)
		mounted_volumes.push_back(MountedVolume{ Volume{ }, GenerateTestSet("", 5, 4), std::move(mp.Path) });

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

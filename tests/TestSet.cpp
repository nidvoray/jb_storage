#include "TestSet.h"

#include <algorithm>
#include <random>
#include <set>

using namespace jb_storage;

namespace
{

	std::random_device random_device;

}

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

			auto value
			{
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

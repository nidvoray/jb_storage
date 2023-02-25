#ifndef STORAGE_TESTS_TESTSET_H
#define STORAGE_TESTS_TESTSET_H

#include "Common.h"

struct TestEntity
{
	std::string			Path;
	jb_storage::Value	Value_;
};

using TestSet = std::vector<TestEntity>;

// note that total size of set (and number of threads as well) is sum([node_power**i for i in range (1,level + 1)])
// please choose arguments so as not to exceed your OS limitation
TestSet GenerateTestSet(const std::string& uplevel_path, size_t node_power, size_t level);

#endif

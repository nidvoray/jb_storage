#include "Path.h"

#include <gtest/gtest.h>

using namespace jb_storage;

namespace
{

	const std::string source{ "/foo/bar/baz/etc" };

}

TEST(PathTest, Valid)
{ ASSERT_NO_THROW(utility::Path{ source }); }

TEST(PathTest, TrailingSlash)
{ ASSERT_THROW(utility::Path{ "/foo/bar/baz/etc/" }, std::invalid_argument); }

TEST(PathTest, InvalidSymbol)
{ ASSERT_THROW(utility::Path{ "/foo/#ar/baz/etc" }, std::invalid_argument); }

TEST(PathTest, Iterators)
{
	const std::vector<std::string> expected{ "foo", "bar", "baz", "etc" };

	{
		std::vector<std::string> result;
		const utility::Path path{ source };
		std::copy(path.begin(), path.end(), std::back_inserter(result));
		ASSERT_EQ(expected, result);
	}

	{
		std::vector<std::string> result;
		for (const auto& key : utility::Path{ source })
			result.emplace_back(key);
		ASSERT_EQ(expected, result);
	}
}

TEST(PathTest, Depth)
{ ASSERT_EQ(utility::Path{ source }.GetDepth(), 4); }

TEST(PathTest, Emptiness)
{
	const std::string source{ "/" };
	ASSERT_TRUE(utility::Path{ source }.IsEmpty());
}

TEST(PathTest, Rest)
{
	const utility::Path path{ source };
	const auto tail{ path.GetRest(std::next(path.begin(), 2)) };

	ASSERT_EQ(tail.GetDepth(), 2);

	const std::vector<std::string> expected{ "baz", "etc" };

	std::vector<std::string> result;
	std::copy(tail.begin(), tail.end(), std::back_inserter(result));
	ASSERT_EQ(expected, result);
}


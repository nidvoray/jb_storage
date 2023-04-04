#include "Path.h"

#include <gtest/gtest.h>

using namespace jb_storage;

namespace
{

	const std::string source{ "/foo///bar/baz/etc//" };

	const auto view2str = [](const auto view) { return std::string{ view }; };

}

TEST(PathTest, Valid)
{ ASSERT_NO_THROW(utility::Path{ source }); }

TEST(PathTest, Relative)
{ ASSERT_THROW(utility::Path{ "foo" }, std::invalid_argument); }

TEST(PathTest, Iterators)
{
	const std::vector<std::string> expected{ "foo", "bar", "baz", "etc" };

	{
		std::vector<std::string> result;
		const utility::Path path{ source };
		std::transform(path.begin(), path.end(), std::back_inserter(result), view2str);
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

TEST(PathTest, IteratorArithmetic)
{
	const utility::Path path{ source };
	auto it{ path.begin() };

	ASSERT_EQ((it++)->at(1), 'o'); // fOo
	ASSERT_EQ(it->back(), 'r'); // baR
	ASSERT_EQ((++it)->front(), 'b'); // Baz
}

TEST(PathTest, Rest)
{
	const utility::Path path{ source };
	const auto tail{ path.GetRest(std::next(path.begin(), 2)) };

	ASSERT_EQ(tail.GetDepth(), 2);

	const std::vector<std::string> expected{ "baz", "etc" };

	std::vector<std::string> result;
	std::transform(tail.begin(), tail.end(), std::back_inserter(result), view2str);
	ASSERT_EQ(expected, result);
}

TEST(PathTest, International)
{
	const std::string source{ "/фу./бар#/баз?/итд*/" };
	const std::vector<std::string> expected{ "фу.", "бар#", "баз?", "итд*" };

	std::vector<std::string> result;
	const utility::Path path{ source };
	std::transform(path.begin(), path.end(), std::back_inserter(result), view2str);
	ASSERT_EQ(expected, result);
}

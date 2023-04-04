#include "PathView.h"

#include <gtest/gtest.h>

using namespace jb_storage;

namespace
{

	const std::string source{ "/foo///bar/baz/etc//" };

	const auto view2str = [](const auto view) { return std::string{ view }; };

}

TEST(PathViewTest, Valid)
{ ASSERT_NO_THROW(utility::PathView{ source }); }

TEST(PathViewTest, Relative)
{ ASSERT_THROW(utility::PathView{ "foo" }, std::invalid_argument); }

TEST(PathViewTest, Iterators)
{
	const std::vector<std::string> expected{ "foo", "bar", "baz", "etc" };

	{
		std::vector<std::string> result;
		const utility::PathView path{ source };
		std::transform(path.begin(), path.end(), std::back_inserter(result), view2str);
		ASSERT_EQ(expected, result);
	}

	{
		std::vector<std::string> result;
		for (const auto& key : utility::PathView{ source })
			result.emplace_back(key);
		ASSERT_EQ(expected, result);
	}
}

TEST(PathViewTest, Depth)
{ ASSERT_EQ(utility::PathView{ source }.GetDepth(), 4); }

TEST(PathViewTest, Emptiness)
{
	const std::string source{ "/" };
	ASSERT_TRUE(utility::PathView{ source }.IsEmpty());
}

TEST(PathViewTest, IteratorArithmetic)
{
	const utility::PathView path{ source };
	auto it{ path.begin() };

	ASSERT_EQ((it++)->at(1), 'o'); // fOo
	ASSERT_EQ(it->back(), 'r'); // baR
	ASSERT_EQ((++it)->front(), 'b'); // Baz
}

TEST(PathViewTest, Rest)
{
	const utility::PathView path{ source };
	const auto tail{ path.GetRest(std::next(path.begin(), 2)) };

	ASSERT_EQ(tail.GetDepth(), 2);

	const std::vector<std::string> expected{ "baz", "etc" };

	std::vector<std::string> result;
	std::transform(tail.begin(), tail.end(), std::back_inserter(result), view2str);
	ASSERT_EQ(expected, result);
}

TEST(PathViewTest, International)
{
	const std::string source{ "/фу./бар#/баз?/итд*/" };
	const std::vector<std::string> expected{ "фу.", "бар#", "баз?", "итд*" };

	std::vector<std::string> result;
	const utility::PathView path{ source };
	std::transform(path.begin(), path.end(), std::back_inserter(result), view2str);
	ASSERT_EQ(expected, result);
}

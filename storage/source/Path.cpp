#include "Path.h"

namespace jb_storage::utility
{

	namespace
	{

		const std::regex validator{ R"(^(?:/)|(?:/\w+)+$)" }; // /foo/bar/baz/etc
		const std::regex tokenizer{ "/" };

	}

	Path::Path(const std::string_view path)
	{
		if (!std::regex_match(path.begin(), path.end(), validator))
			throw std::invalid_argument{ path.data() };

		_begin = std::next(const_iterator{ path.begin(), path.end(), tokenizer, -1 }); //skip empty root
		_depth = std::distance(_begin, _end);
	}

	Path::Path(const Path& whole, const const_iterator& current)
		: _begin{ current }, _end{ whole._end }, _depth{ static_cast<size_t>(std::distance(_begin, _end)) }
	{ }

}

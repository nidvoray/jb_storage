#include "Path.h"

namespace jb_storage::utility
{

	namespace
	{

		const std::regex validator{ R"(^(?:/)|(?:/\w+)+$)" }; // /foo/bar/baz/etc
		const std::regex tokenizer{ "/" };

	}

	Path::Path(const std::string& path)
		: _path{ path }
	{
		if (!std::regex_match(path, validator))
			throw std::invalid_argument{ path };

		_begin = std::next(const_iterator{ _path.begin(), _path.end(), tokenizer, -1 }); //skip empty root
		_depth = std::distance(_begin, _end);
	}

	Path::Path(const Path& whole, const const_iterator& current)
		: _path{ whole._path }, _begin{ current }, _end{ whole._end }, _depth{ static_cast<size_t>(std::distance(_begin, _end)) }
	{ }

}

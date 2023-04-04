#include "Path.h"

#include <stdexcept>

namespace jb_storage::utility
{

	Path::Path(const std::string_view path)
		: _begin{ path }, _end{ path.substr(path.length()) }, _depth{ static_cast<size_t>(std::distance(_begin, _end)) }
	{
		if (path.front() != s_separator)
			throw std::invalid_argument{ path.data() };
	}

	Path::Path(const Path& whole, const const_iterator& current)
		: _begin{ current }, _end{ whole._end }, _depth{ static_cast<size_t>(std::distance(_begin, _end)) }
	{ }

}

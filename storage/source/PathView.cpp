#include "PathView.h"

#include <stdexcept>

namespace jb_storage::utility
{

	PathView::PathView(const std::string_view path)
		: _begin{ path }, _end{ path.substr(path.length()) }, _depth{ static_cast<size_t>(std::distance(_begin, _end)) }
	{
		if (path.front() != s_separator)
			throw std::invalid_argument{ path.data() };
	}

	PathView::PathView(const PathView& whole, const const_iterator& current)
		: _begin{ current }, _end{ whole._end }, _depth{ static_cast<size_t>(std::distance(_begin, _end)) }
	{ }

}

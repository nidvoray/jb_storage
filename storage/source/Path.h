#ifndef STORAGE_PATH_H
#define STORAGE_PATH_H

#include <regex>
#include <string_view>

namespace jb_storage::utility
{

	class Path final
	{
	public:
		using const_iterator = std::cregex_token_iterator;

	private:
		const_iterator		_begin;
		const_iterator		_end;
		size_t				_depth;

	public:
		explicit Path(const std::string_view path);

		size_t GetDepth() const noexcept 				{ return _depth; }
		bool IsEmpty() const noexcept 					{ return GetDepth() == 0; }
		Path GetRest(const const_iterator& it) const	{ return Path{ *this, it }; }

		const_iterator cbegin() const noexcept			{ return _begin; }
		const_iterator cend() const noexcept			{ return _end; }
		const_iterator begin() const noexcept			{ return cbegin(); }
		const_iterator end() const noexcept				{ return cend(); }

	private:
		Path(const Path& whole, const const_iterator& current);
	};

}

#endif

#ifndef STORAGE_PATH_H
#define STORAGE_PATH_H

#include <algorithm>
#include <iterator>
#include <string_view>

namespace jb_storage::utility
{

	class PathView final
	{
	public:
		class const_iterator final
		{
			friend class PathView;

		public:
			using value_type = std::string_view;
			using difference_type = std::ptrdiff_t;
			using reference = value_type;
			using iterator_category = std::input_iterator_tag;

			class PtrProxy final
			{
				friend class const_iterator;

			private:
				value_type	_val;

			public:
				const value_type* operator -> () const noexcept { return &_val; }

			private:
				explicit PtrProxy(value_type&& val) noexcept : _val{ std::move(val) } { }
			};

			using pointer = PtrProxy;

		private:
			std::string_view	_rest;
			std::string_view	_current;

		public:
			reference operator * () const noexcept { return _current; }
			pointer operator -> () const noexcept { return PtrProxy{ **this }; }
			const_iterator& operator ++ () { return *this = const_iterator{ _rest.substr(_current.length()) }; }
			const_iterator operator ++ (int) { const auto copy{ *this }; ++(*this); return copy; }
			bool operator == (const const_iterator& other) const noexcept { return _current.data() == other._current.data(); }
			bool operator != (const const_iterator& other) const noexcept { return !(*this == other); }

		private:
			explicit const_iterator(const std::string_view path)
				:	_rest{ path.substr(std::min(path.length(), path.find_first_not_of(s_separator))) },
					_current{ _rest.substr(0, std::min(_rest.length(), _rest.find_first_of(s_separator))) }
			{ }
		};

	private:
		static constexpr char s_separator{ '/' };

		const_iterator		_begin;
		const_iterator		_end;
		size_t				_depth;

	public:
		explicit PathView(const std::string_view path);

		size_t GetDepth() const noexcept 					{ return _depth; }
		bool IsEmpty() const noexcept 						{ return GetDepth() == 0; }
		PathView GetRest(const const_iterator& it) const	{ return PathView{ *this, it }; }

		const_iterator cbegin() const noexcept				{ return _begin; }
		const_iterator cend() const noexcept				{ return _end; }
		const_iterator begin() const noexcept				{ return cbegin(); }
		const_iterator end() const noexcept					{ return cend(); }

	private:
		PathView(const PathView& whole, const const_iterator& current);
	};

}

#endif

#ifndef STORAGE_TRACELOCKER_H
#define STORAGE_TRACELOCKER_H

#include <shared_mutex>
#include <vector>

namespace jb_storage::utility
{

	template < typename LockType >
	class TraceLocker final
	{
	private:
		std::vector<std::shared_lock<LockType>>	_trace;
		size_t									_depth;

	public:
		explicit TraceLocker(size_t depth)
			: _depth{ depth }
		{ _trace.reserve(depth); }

		TraceLocker(const TraceLocker&) = delete;
		TraceLocker& operator =(const TraceLocker&) = delete;

		TraceLocker(TraceLocker&&) = default;
		TraceLocker& operator =(TraceLocker&&) = default;

		~TraceLocker()
		{  while (!_trace.empty()) _trace.pop_back(); }

		void Push(LockType& obj)
		{
			if (_trace.size() == _depth)
				throw std::out_of_range{ "trace depth " + std::to_string(_depth) + " exceeded!" };

			_trace.push_back(std::shared_lock{ obj });
		}

		void Pop()
		{
			if (_trace.empty())
				throw std::out_of_range{ "popping from empty trace" };

			_trace.pop_back();
		}
	};

}

#endif

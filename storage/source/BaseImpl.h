#ifndef STORAGE_BASEIMPL_H
#define STORAGE_BASEIMPL_H

#include "INode.h"
#include "Path.h"
#include "TraceLocker.h"

#include <utility>

namespace jb_storage
{

	class BaseImpl
	{
	private:
		INodePtr	_root;

	public:
		std::optional<Value> Get(const std::string& path) const;
		bool Delete(const std::string& path) const;
		bool SetOrInsert(const std::string& path, const Value& value) const;

	protected:
		explicit BaseImpl(const INodePtr& root) noexcept : _root{ root } { }

		std::pair<INodePtr, utility::TraceLocker> LockPath(const std::string& path) const;

		template <typename NodePointerType>
		static bool GrowBranchAndSetValue(
				const NodePointerType& root,
				const std::string& path_,
				const std::function<NodePointerType (const NodePointerType&, const std::string& name)>& child_getter,
				const std::function<bool (const NodePointerType&, const utility::Path& path)>& value_setter)
		{
			const utility::Path path{ path_ };
			utility::TraceLocker locker{ path.GetDepth() };

			auto current{ root };
			auto key{ path.begin() };
			const auto end{ path.end() };

			do
			{
				for (; key != end; ++key)
				{
					locker.Push(current);
					if (const auto child{ child_getter(current, *key) })
						current = child;
					else
						break;
				}

				locker.Pop();

				std::unique_lock lock{ *current };

				if (key != end && child_getter(current, *key))
					continue;

				return value_setter(current, path.GetRest(key));
			}
			while (key != end);

			return true;
		}
	};

}

#endif

#ifndef STORAGE_BASEIMPL_H
#define STORAGE_BASEIMPL_H

#include "INode.h"
#include "PathView.h"

#include <mutex>
#include <shared_mutex>
#include <utility>

namespace jb_storage
{

	class BaseImpl
	{
	private:
		INodePtr	_root;

	public:
		std::optional<Value> Get(const std::string_view path) const;
		bool Delete(const std::string_view path) const;
		bool SetOrInsert(const std::string_view path, Value&& value) const;

	protected:
		explicit BaseImpl(const INodePtr& root) noexcept : _root{ root } { }

		INodePtr GetNode(const std::string_view path) const;

		template < typename NodePointerType, typename LockAdaptor = typename NodePointerType::element_type, typename ChildGetter, typename ValueSetter >
		static bool GrowBranchAndSetValue(
				const NodePointerType& root,
				const std::string_view path_,
				ChildGetter&& child_getter,
				ValueSetter&& value_setter)
		{
			const utility::PathView path{ path_ };

			NodePointerType current{ root };
			auto key{ path.begin() };
			const auto end{ path.end() };

			do
			{
				for (; key != end; ++key)
				{
					std::shared_lock lock{ *current };
					if (const NodePointerType child{ child_getter(current, *key) })
						current = child;
					else
						break;
				}

				std::unique_lock lock{ static_cast<LockAdaptor&>(*current) };

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

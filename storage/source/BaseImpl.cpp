#include "BaseImpl.h"

namespace jb_storage
{

	std::optional<Value> BaseImpl::Get(const std::string& path) const
	{
		const auto [node, locker] { LockPath(path) };
		return node ? node->GetValue() : std::nullopt;
	}

	bool BaseImpl::Delete(const std::string& path_) const
	{
		const utility::Path path{ path_ };
		if (!path.GetDepth())
			return false;

		utility::TraceLocker locker{ path.GetDepth() };

		INodePtr parent;
		auto current{ _root };
		std::string key_name;

		for (auto key{ path.begin() }, end{ path.end() }; key != end && current; ++key)
		{
			locker.Push(current);
			parent = current;
			current = current->GetChild(key_name = *key);
		}

		if (!current)
			return false;

		locker.Pop();

		std::unique_lock lock{ *parent };

		return parent->DeleteChild(key_name);
	}

	bool BaseImpl::SetOrInsert(const std::string& path, const Value& value) const
	{
		return GrowBranchAndSetValue<INodePtr>(
				_root,
				path,
				[](const auto& node, const auto& name) { return node->GetChild(name); },
				[&value](const auto& node, const auto& path) { return node->GrowBranchAndSetValue(path, value); });
	}

	std::pair<INodePtr, utility::TraceLocker> BaseImpl::LockPath(const std::string& path_) const
	{
		const utility::Path path{ path_ };
		utility::TraceLocker locker{ path.GetDepth() };

		auto current{ _root };
		for (auto key{ path.begin() }, end{ path.end() }; key != end && current; ++key)
		{
			locker.Push(current);
			current = current->GetChild(*key);
		}

		if (current)
		{
			locker.Push(current);
			return std::make_pair(current, std::move(locker));
		}

		return std::make_pair(nullptr, utility::TraceLocker{ 0 });
	}

}

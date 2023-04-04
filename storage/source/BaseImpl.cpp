#include "BaseImpl.h"

namespace jb_storage
{

	std::optional<Value> BaseImpl::Get(const std::string_view path) const
	{
		const auto [node, locker] { LockPath(path) };
		return node ? node->GetValue() : std::nullopt;
	}

	bool BaseImpl::Delete(const std::string_view path_) const
	{
		const utility::PathView path{ path_ };
		if (!path.GetDepth())
			return false;

		utility::TraceLocker<INode> locker{ path.GetDepth() };

		INodePtr parent;
		auto current{ _root };
		std::string_view key_name;

		for (auto key{ path.begin() }, end{ path.end() }; key != end && current; ++key)
		{
			locker.Push(*current);
			parent = current;
			current = current->GetChild(key_name = *key);
		}

		if (!current)
			return false;

		locker.Pop();

		std::unique_lock lock{ *parent };

		return parent->DeleteChild(key_name);
	}

	bool BaseImpl::SetOrInsert(const std::string_view path, const Value& value) const
	{
		return GrowBranchAndSetValue(
				_root,
				path,
				[](const INodePtr& node, const std::string_view name) { return node->GetChild(name); },
				[&value](const INodePtr& node, const auto& path) { return node->GrowBranchAndSetValue(path, value); });
	}

	std::pair<INodePtr, utility::TraceLocker<INode>> BaseImpl::LockPath(const std::string_view path_) const
	{
		const utility::PathView path{ path_ };
		utility::TraceLocker<INode> locker{ path.GetDepth() + 1 };

		auto current{ _root };
		for (auto key{ path.begin() }, end{ path.end() }; key != end && current; ++key)
		{
			locker.Push(*current);
			current = current->GetChild(*key);
		}

		if (current)
		{
			locker.Push(*current);
			return std::make_pair(current, std::move(locker));
		}

		return std::make_pair(nullptr, utility::TraceLocker<INode>{ 0 });
	}

}

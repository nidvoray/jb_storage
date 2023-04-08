#include "BaseImpl.h"

namespace jb_storage
{

	std::optional<Value> BaseImpl::Get(const std::string_view path) const
	{
		if (const INodePtr node{ GetNode(path) })
		{
			std::shared_lock lock{ *node };
			return node->GetValue();
		}

		return std::nullopt;
	}

	bool BaseImpl::Delete(const std::string_view path_) const
	{
		const utility::PathView path{ path_ };
		if (!path.GetDepth())
			return false;

		INodePtr parent;
		INodePtr current{ _root };
		std::string_view key_name;

		for (auto key{ path.begin() }, end{ path.end() }; key != end && current; ++key)
		{
			std::shared_lock lock{ *current };
			parent = current;
			current = current->GetChild(key_name = *key);
		}

		if (!current)
			return false;

		std::unique_lock lock{ *parent };

		return parent->DeleteChild(key_name);
	}

	bool BaseImpl::SetOrInsert(const std::string_view path, Value&& value) const
	{
		return GrowBranchAndSetValue(
				_root,
				path,
				[](const INodePtr& node, const std::string_view name) { return node->GetChild(name); },
				[&value](const INodePtr& node, const utility::PathView& path) { return node->GrowBranchAndSetValue(path, std::move(value)); });
	}

	INodePtr BaseImpl::GetNode(const std::string_view path_) const
	{
		const utility::PathView path{ path_ };

		INodePtr current{ _root };
		for (auto key{ path.begin() }, end{ path.end() }; key != end && current; ++key)
		{
			std::shared_lock lock{ *current };
			current = current->GetChild(*key);
		}

		return current;
	}

}

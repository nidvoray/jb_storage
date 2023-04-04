#ifndef STORAGE_SOURCE_INODE_H
#define STORAGE_SOURCE_INODE_H

#include "Common.h"
#include "Path.h"

#include <memory>
#include <optional>

namespace jb_storage
{

	using INodePtr = std::shared_ptr<struct INode>;

	struct INode
	{
		virtual ~INode() = default;
		INode() = default;

		INode(const INode&) = delete;
		INode& operator = (const INode&) = delete;

		virtual std::optional<Value> GetValue() const = 0;
		virtual bool GrowBranchAndSetValue(const utility::Path& path, const Value& value) = 0;

		virtual INodePtr GetChild(const std::string_view name) const = 0;
		virtual bool DeleteChild(const std::string_view name) = 0;

		virtual void lock() = 0;
		virtual void unlock() = 0;
		virtual void lock_shared() = 0;
		virtual void unlock_shared() = 0;
	};

}

#endif

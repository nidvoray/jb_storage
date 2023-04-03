#ifndef STORAGE_ISTORAGE_H
#define STORAGE_ISTORAGE_H

#include <Common.h>

#include <optional>
#include <string_view>

namespace jb_storage
{

	struct IStorage
	{
		virtual ~IStorage() = default;
		IStorage() = default;

		IStorage(const IStorage&) = delete;
		IStorage& operator = (const IStorage&) = delete;

		IStorage(IStorage&&) = default;
		IStorage& operator = (IStorage&&) = default;

		virtual std::optional<Value> Get(const std::string_view path) const = 0;
		virtual bool SetOrInsert(const std::string_view path, const Value& value) const = 0;
		virtual bool Delete(const std::string_view path) const = 0;
	};

}

#endif

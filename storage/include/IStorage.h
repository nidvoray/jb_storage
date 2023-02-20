#ifndef STORAGE_ISTORAGE_H
#define STORAGE_ISTORAGE_H

#include <Common.h>

#include <optional>

namespace jb_storage
{

	struct IStorage
	{
		virtual ~IStorage() = default;
		IStorage() = default;

		IStorage(const IStorage&) = delete;
		IStorage& operator = (const IStorage&) = delete;

		virtual std::optional<Value> Get(const std::string& path) const = 0;
		virtual bool SetOrInsert(const std::string& path, const Value& value) const = 0;
		virtual bool Delete(const std::string& path) const = 0;
	};

}

#endif

#ifndef STORAGE_VOLUME_H
#define STORAGE_VOLUME_H

#include "IStorage.h"

#include <istream>
#include <memory>
#include <ostream>

namespace jb_storage
{

	class VolumeImpl;

	class Volume final : public IStorage
	{
		friend class Storage;

	private:
		std::shared_ptr<VolumeImpl>	_impl;

	public:
		Volume();

		std::optional<Value> Get(const std::string_view path) const override;
		bool SetOrInsert(const std::string_view path, const Value& value) const override;
		bool SetOrInsert(const std::string_view path, Value&& value) const override;
		bool Delete(const std::string_view path) const override;

		bool Load(std::istream& is) const;
		bool Save(std::ostream& os) const;
	};

}

#endif

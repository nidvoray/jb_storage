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

		std::optional<Value> Get(const std::string& path) const override;
		bool SetOrInsert(const std::string& path, const Value& value) const override;
		bool Delete(const std::string& path) const override;

		void Load(std::istream& is) const;
		void Save(std::ostream& os) const;
	};

}

#endif

#ifndef STORAGE_STORAGE_H
#define STORAGE_STORAGE_H

#include "Volume.h"

namespace jb_storage
{

	class Storage final : public IStorage
	{
		class Impl;
		class MountTokenImpl;

	public:
		class MountToken final
		{
			friend class Storage;
			using MountTokenImplPtr = std::shared_ptr<MountTokenImpl>;

		private:
			MountTokenImplPtr	_impl;

		public:
			explicit operator bool () const noexcept;

		private:
			explicit MountToken(const MountTokenImplPtr& impl) noexcept;
		};

	private:
		std::unique_ptr<Impl>	_impl;

	public:
		Storage();
		~Storage();

		std::optional<Value> Get(const std::string_view path) const override;
		bool SetOrInsert(const std::string_view path, const Value& value) const override;
		bool Delete(const std::string_view path) const override;

		MountToken Mount(const std::string_view where, const Volume& volume, const std::string_view what) const;
	};

}

#endif

#ifndef STORAGE_VOLUMEIMPL_H
#define STORAGE_VOLUMEIMPL_H

#include "BaseImpl.h"

#include <atomic>
#include <istream>
#include <ostream>

namespace jb_storage
{

	class VolumeImpl final : public BaseImpl
	{
	private:
		INodePtr				_root;
		std::atomic<unsigned>	_refcounter;

	public:
		VolumeImpl();

		using BaseImpl::LockPath;

		void AddRef();
		void Release();

		void Load(std::istream& is) const;
		void Save(std::ostream& os) const;
	};

	using VolumeImplPtr = std::shared_ptr<VolumeImpl>;

}

#endif

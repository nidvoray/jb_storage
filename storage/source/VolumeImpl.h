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
		class Node;
		using NodePtr = std::shared_ptr<Node>;

	private:
		NodePtr					_root;
		std::atomic<unsigned>	_refcounter;

	public:
		VolumeImpl();

		using BaseImpl::GetNode;

		void AddRef() noexcept;
		void Release() noexcept;

		bool Load(std::istream& is) const;
		bool Save(std::ostream& os) const;

	private:
		explicit VolumeImpl(NodePtr&& root) noexcept;

		bool IsUsed() const noexcept;
	};

	using VolumeImplPtr = std::shared_ptr<VolumeImpl>;

}

#endif

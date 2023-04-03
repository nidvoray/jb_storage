#include "Storage.h"

#include "Mutex.h"
#include "VolumeImpl.h"

#include <iostream>
#include <map>

namespace jb_storage
{

	namespace
	{

		class MountHolder final
		{
		private:
			INodePtr					_node;
			std::weak_ptr<VolumeImpl>	_volume;

		public:
			MountHolder(const INodePtr& node, const VolumeImplPtr& volume) noexcept
				: _node{ node }, _volume{ volume }
			{ volume->AddRef(); }

			~MountHolder()
			{
				if (const auto volume{ _volume.lock() })
					volume->Release();
			}

			 INodePtr GetNode() const noexcept { return _node; }
		};

		using MountHolderPtr = std::shared_ptr<MountHolder>;
		using MountHolderWeakPtr = std::weak_ptr<MountHolder>;

		class VirtualNodeNonPolymorphicLockMixin
		{
		private:
			MutexType	_lock;

		public:
			void lock()				{ _lock.lock(); }
			void unlock()			{ _lock.unlock(); }
			void lock_shared()		{ _lock.lock_shared();}
			void unlock_shared()	{ _lock.unlock_shared();}

		};

		using VirtualNodePtr = std::shared_ptr<class VirtualNode>;

		class VirtualNode final : public INode, public VirtualNodeNonPolymorphicLockMixin
		{
			using NonPolymorphicBase = VirtualNodeNonPolymorphicLockMixin;

		private:
			std::vector<MountHolderPtr>				_mounted;
			std::map<std::string, VirtualNodePtr>	_virtual_children;

		public:
			std::optional<Value> GetValue() const override
			{
				if (!_mounted.empty())
					return _mounted.back()->GetNode()->GetValue();

				return std::nullopt;
			}

			bool GrowBranchAndSetValue(const utility::Path& path, const Value& value) override
			{
				if (!_mounted.empty())
					return _mounted.back()->GetNode()->GrowBranchAndSetValue(path, value);

				return false;
			}

			INodePtr GetChild(const std::string& name) const override
			{
				for (auto rmounted{ _mounted.rbegin() }, rend{ _mounted.rend() }; rmounted != rend; ++rmounted)
					if (const auto child{ (*rmounted)->GetNode()->GetChild(name) })
						return child;

				return GetVirtualChild(name);
			}

			bool DeleteChild(const std::string& name) override
			{
				for (auto rmounted{ _mounted.rbegin() }, rend{ _mounted.rend() }; rmounted != rend; ++rmounted)
					if ((*rmounted)->GetNode()->DeleteChild(name))
						return true;

				return _virtual_children.erase(name) != 0;
			}

			void lock() override
			{
				NonPolymorphicBase::lock();
				std::for_each(_mounted.begin(), _mounted.end(), [](const auto& mounted) { mounted->GetNode()->lock(); } );
			}

			void unlock() override
			{
				std::for_each(_mounted.rbegin(), _mounted.rend(), [](const auto& mounted) { mounted->GetNode()->unlock(); } );
				NonPolymorphicBase::unlock();
			}

			void lock_shared() override
			{
				NonPolymorphicBase::lock_shared();
				std::for_each(_mounted.begin(), _mounted.end(), [](const auto& mounted) { mounted->GetNode()->lock_shared(); } );
			}

			void unlock_shared() override
			{
				std::for_each(_mounted.rbegin(), _mounted.rend(), [](const auto& mounted) { mounted->GetNode()->unlock_shared(); } );
				NonPolymorphicBase::unlock_shared();
			}

			VirtualNodePtr GrowBranchAndMount(const utility::Path& path, const MountHolderPtr& holder)
			{
				if (!path.IsEmpty())
				{
					auto key{ path.begin() };

					const auto new_subbranch{ std::make_shared<VirtualNode>() };
					const auto new_subbranch_name{ *key++ };

					auto tail{ new_subbranch };
					for (const auto end{ path.end() }; key != end; ++key)
						tail = tail->SetVirtualChild(*key, std::make_shared<VirtualNode>());

					tail->Mount(holder);

					SetVirtualChild(new_subbranch_name, new_subbranch);

					return tail;
				}
				else
					Mount(holder);

				return nullptr; //avoid using shared_from_this() here
			}

			VirtualNodePtr GetVirtualChild(const std::string& name) const
			{
				const auto child{ _virtual_children.find(name) };
				return child != _virtual_children.end() ? child->second : nullptr;
			}

			void Unmount(const MountHolderWeakPtr& holder_weak)
			{
				if (const auto holder{ holder_weak.lock() })
					if (const auto found{ std::find(_mounted.begin(), _mounted.end(), holder) }; found != _mounted.end())
						_mounted.erase(found);
			}

		private:
			VirtualNodePtr SetVirtualChild(const std::string& name, const VirtualNodePtr& child)
			{
				_virtual_children.insert_or_assign(name, child);
				return child;
			}

			void Mount(const MountHolderPtr& holder)
			{ _mounted.push_back(holder); }
		};

	}

	class Storage::Impl final : public BaseImpl
	{
		using MountTokenImplPtr = MountToken::MountTokenImplPtr;

	private:
		VirtualNodePtr	_root;

	public:
		Impl() : Impl{ std::make_shared<VirtualNode>() } { };

		MountTokenImplPtr Mount(const std::string_view where, const VolumeImplPtr& volume, const std::string_view what) const;

	private:
		Impl(const VirtualNodePtr& root) noexcept : BaseImpl{ root }, _root{ root } { }
	};

	class Storage::MountTokenImpl final
	{
	private:
		std::weak_ptr<VirtualNode>	_owner;
		MountHolderWeakPtr			_holder;

	public:
		MountTokenImpl(const VirtualNodePtr& owner, const MountHolderPtr& holder) noexcept
			: _owner{ owner }, _holder{ holder }
		{ }

		~MountTokenImpl()
		{
			if (const auto owner{ _owner.lock() })
			try
			{
				std::unique_lock lock{ static_cast<VirtualNodeNonPolymorphicLockMixin&>(*owner) };
				owner->Unmount(_holder);
			}
			catch (const std::system_error& e)
			{ std::cerr << "this could never happen but std::system_error with code " << e.code() << " meaning " << e.what() << " has been caught\n"; }
		}
	};

	Storage::Impl::MountTokenImplPtr Storage::Impl::Mount(const std::string_view where, const VolumeImplPtr& volume, const std::string_view what) const
	{
		if (const auto [volume_node, locker] { volume->LockPath(what) }; volume_node)
		{
			const auto holder{ std::make_shared<MountHolder>(volume_node, volume) };
			VirtualNodePtr owner;

			GrowBranchAndSetValue<VirtualNodePtr, VirtualNodeNonPolymorphicLockMixin>(
					_root,
					where,
					[](const VirtualNodePtr& storage_node, const auto& name) { return storage_node->GetVirtualChild(name); },
					[&holder, &owner](const VirtualNodePtr& storage_node, const auto& path)
					{
						const auto retval{ storage_node->GrowBranchAndMount(path, holder) };
						owner = retval ? retval : storage_node; //workaround for returning shared_from_this() from GrowBranchAndMount()
						return true;
					});

			return std::make_shared<MountTokenImpl>(owner, holder);
		}

		return nullptr;
	}

	Storage::MountToken::operator bool () const noexcept
	{ return !!_impl; }

	Storage::MountToken::MountToken(const MountTokenImplPtr& impl) noexcept
		: _impl{ impl }
	{ }

	Storage::Storage()
		: _impl{ std::make_unique<Impl>() }
	{ }

	Storage::~Storage() = default;

	std::optional<Value> Storage::Get(const std::string& path) const
	{ return _impl->Get(path); }

	bool Storage::SetOrInsert(const std::string& path, const Value& value) const
	{ return _impl->SetOrInsert(path, value); }

	bool Storage::Delete(const std::string& path) const
	{ return _impl->Delete(path); }

	Storage::MountToken Storage::Mount(const std::string& where, const Volume& volume, const std::string& what) const
	{ return MountToken{ _impl->Mount(where, volume._impl, what) }; }

}

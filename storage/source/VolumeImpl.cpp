#include "VolumeImpl.h"

#include "Mutex.h"

#include <map>

namespace jb_storage
{

	namespace
	{

		class Node final : public INode
		{
			using NodePtr = std::shared_ptr<Node>;

		private:
			Value							_value;
			std::map<std::string, INodePtr>	_children;
			MutexType						_lock;

		public:
			std::optional<Value> GetValue() const override
			{ return _value; }

			bool GrowBranchAndSetValue(const utility::Path& path, const Value& value) override
			{
				if (!path.IsEmpty())
				{
					auto key{ path.begin() };

					const auto new_subbranch{ std::make_shared<Node>() };
					const auto new_subbranch_name{ *key++ };

					auto tail{ new_subbranch };
					for (const auto end{ path.end() }; key != end; ++key)
						tail = tail->SetChild(*key, std::make_shared<Node>());

					tail->_value = value;

					SetChild(new_subbranch_name, new_subbranch);
				}
				else
					_value = value;

				return true;
			}

			INodePtr GetChild(const std::string& name) const override
			{
				const auto child{ _children.find(name) };
				return child != _children.end() ? child->second : nullptr;
			}

			bool DeleteChild(const std::string& name) override
			{ return _children.erase(name) != 0; }

			void lock() override
			{ _lock.lock(); }

			void unlock() override
			{ _lock.unlock(); }

			void lock_shared() override
			{ _lock.lock_shared(); }

			void unlock_shared() override
			{ _lock.unlock_shared(); }

		private:
			NodePtr SetChild(const std::string& name, const NodePtr& child)
			{
				_children.insert_or_assign(name, child);
				return child;
			}
		};

	}

	VolumeImpl::VolumeImpl()
		: BaseImpl{ std::make_shared<Node>() }
	{ }

	void VolumeImpl::AddRef()
	{ _refcounter.fetch_add(1, std::memory_order_relaxed); }

	void VolumeImpl::Release()
	{ _refcounter.fetch_sub(1, std::memory_order_relaxed); }

	void VolumeImpl::Load(std::istream& is) const
	{
		if (_refcounter.load(std::memory_order_acquire) == 0)
		{
			std::unique_lock lock{ *_root };
			// TODO: load data here
		}
	}

	void VolumeImpl::Save(std::ostream& os) const
	{
		if (_refcounter.load(std::memory_order_acquire) == 0)
		{
			std::unique_lock lock{ *_root };
			// TODO: save data here
		}
	}

}

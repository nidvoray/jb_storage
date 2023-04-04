#include "VolumeImpl.h"

#include "Mutex.h"
#include "Serialization.h"

#include <map>

namespace jb_storage
{

	class VolumeImpl::Node final : public INode
	{
	private:
		Value										_value;
		std::map<std::string, NodePtr, std::less<>>	_children;
		MutexType									_lock;

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

		INodePtr GetChild(const std::string_view name) const override
		{
			const auto child{ _children.find(name) };
			return child != _children.end() ? child->second : nullptr;
		}

		bool DeleteChild(const std::string_view name) override
		{
			// std::map::erase with equivalent key comparison appears in c++23 only
			if (const auto child{ _children.find(name) }; child != _children.end())
			{
				_children.erase(child);
				return true;
			}

			return false;
		}

		void lock() override
		{ _lock.lock(); }

		void unlock() override
		{ _lock.unlock(); }

		void lock_shared() override
		{ _lock.lock_shared(); }

		void unlock_shared() override
		{ _lock.unlock_shared(); }

		void swap(Node& other) noexcept
		{
			_value.swap(other._value);
			_children.swap(other._children);
		}

		void Serialize(std::ostream& os) const
		{
			utility::Serialize(_value, os);
			utility::Serialize(static_cast<uint64_t>(_children.size()), os);

			for (const auto& child : _children)
			{
				utility::Serialize(child.first, os);
				child.second->Serialize(os);
			}
		}

		void Deserialize(std::istream& is)
		{
			Node node;
			node._value = utility::Deserialize<Value>(is);

			const auto count{ utility::Deserialize<uint64_t>(is) };
			for (uint64_t i{ 0 }; i < count; ++i)
			{
				const auto name{ utility::Deserialize<std::string>(is) };
				const auto child{ std::make_shared<Node>() };
				child->Deserialize(is);
				node.SetChild(name, child);
			}

			swap(node);
		}

	private:
		NodePtr SetChild(const std::string_view name, const NodePtr& child)
		{
			_children.insert_or_assign(std::string{ name }, child);
			return child;
		}
	};

	VolumeImpl::VolumeImpl()
		: VolumeImpl{ std::make_shared<Node>() }
	{ }

	void VolumeImpl::AddRef() noexcept
	{ _refcounter.fetch_add(1, std::memory_order_release); }

	void VolumeImpl::Release() noexcept
	{ _refcounter.fetch_sub(1, std::memory_order_release); }

	bool VolumeImpl::Load(std::istream& is) const
	{
		if (IsUsed())
			return false;

		std::unique_lock lock{ *_root };

		if (IsUsed())
			return false;

		const auto saved_state{ is.exceptions() };
		is.exceptions(std::ios::failbit | std::ios::badbit);

		bool status{ true };
		try
		{
			const auto creature{ std::make_shared<Node>() };
			creature->Deserialize(is);
			_root->swap(*creature);
		}
		catch (const std::exception&)
		{ status = false; }

		is.exceptions(saved_state);

		return status;
	}

	bool VolumeImpl::Save(std::ostream& os) const
	{
		if (IsUsed())
			return false;

		std::unique_lock lock{ *_root };

		if (IsUsed())
			return false;

		const auto saved_state{ os.exceptions() };
		os.exceptions(std::ios::failbit | std::ios::badbit);

		bool status{ true };
		try
		{ _root->Serialize(os); }
		catch (const std::exception&)
		{ status = false; }

		os.exceptions(saved_state);

		return status;
	}

	VolumeImpl::VolumeImpl(const NodePtr& root) noexcept
		: BaseImpl{ root }, _root{ root }, _refcounter{ 0 }
	{ }

	bool VolumeImpl::IsUsed() const noexcept
	{ return _refcounter.load(std::memory_order_acquire) != 0; }

}

#include "Volume.h"

#include "VolumeImpl.h"

namespace jb_storage
{

	Volume::Volume()
		: _impl{ std::make_shared<VolumeImpl>() }
	{ }

	std::optional<Value> Volume::Get(const std::string_view path) const
	{ return _impl->Get(path); }

	bool Volume::SetOrInsert(const std::string_view path, const Value& value) const
	{ return _impl->SetOrInsert(path, Value{ value }); }

	bool Volume::SetOrInsert(const std::string_view path, Value&& value) const
	{ return _impl->SetOrInsert(path, std::move(value)); }

	bool Volume::Delete(const std::string_view path) const
	{ return _impl->Delete(path); }

	bool Volume::Load(std::istream& is) const
	{ return _impl->Load(is); }

	bool Volume::Save(std::ostream& os) const
	{ return _impl->Save(os); }

}

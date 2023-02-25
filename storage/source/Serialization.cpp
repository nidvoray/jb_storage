#include "Serialization.h"

namespace jb_storage::utility
{

	namespace
	{

		enum class ValueType
		{
			Uint32,
			Uint64,
			Float,
			Double,
			String,
			Blob,
			Monostate
		};

	}

	void Serialize(const Value& val, std::ostream& os)
	{
		std::visit([&os](const auto& arg)
		{
			using T = std::decay_t<decltype(arg)>;
			constexpr auto type
			{
				std::is_same_v<T, uint32_t> ? ValueType::Uint32 :
				std::is_same_v<T, uint64_t> ? ValueType::Uint64 :
				std::is_same_v<T, float> ? ValueType::Float :
				std::is_same_v<T, double> ? ValueType::Double :
				std::is_same_v<T, std::string> ? ValueType::String :
				std::is_same_v<T, Blob> ? ValueType::Blob :
				ValueType::Monostate
			};

			Serialize(static_cast<uint8_t>(type), os);
			if (type != ValueType::Monostate)
				Serialize(arg, os);
		}, val);
	}

	template < >
	Value Deserialize<Value>(std::istream& is)
	{
		const auto type{ static_cast<ValueType>(Deserialize<uint8_t>(is)) };

		return
			type == ValueType::Uint32 ? Value{ Deserialize<uint32_t>(is) } :
			type == ValueType::Uint64 ? Value{ Deserialize<uint64_t>(is) } :
			type == ValueType::Float ? Value{ Deserialize<float>(is) } :
			type == ValueType::Double ? Value{ Deserialize<double>(is) } :
			type == ValueType::String ? Value{ Deserialize<std::string>(is) } :
			type == ValueType::Blob ? Value{ Deserialize<Blob>(is) } :
			Value{ };
	}

}

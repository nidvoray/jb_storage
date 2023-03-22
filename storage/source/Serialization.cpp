#include "Serialization.h"

namespace jb_storage::utility
{

	namespace
	{

		template < typename... T >
		std::variant<T...> Deserialize(std::istream& is, const std::variant<T...>*)
		{
			using Value = std::variant<T...>;

			const size_t index{ utility::Deserialize<uint8_t>(is) };
			if (index >= sizeof...(T))
				throw std::out_of_range{ "index " + std::to_string(index) + " out of range" };

			static Value (* const creators[])(std::istream&)
			{
				[](std::istream& is)
				{
					if constexpr (!std::is_same_v<T, std::monostate>)
						return Value{ utility::Deserialize<T>(is) };
					else
						return Value{ };
				}...
			};

			return creators[index](is);
		}

	}

	void Serialize(const Value& val, std::ostream& os)
	{
		Serialize(static_cast<uint8_t>(val.index()), os);
		std::visit([&os](const auto& arg)
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr (!std::is_same_v<T, std::monostate>)
				Serialize(arg, os);
		}, val);
	}

	template < >
	Value Deserialize<Value>(std::istream& is)
	{ return Deserialize(is, static_cast<const Value*>(nullptr)); }

}

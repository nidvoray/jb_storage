#ifndef STORAGE_SERIALIZATION_H
#define STORAGE_SERIALIZATION_H

#include <Common.h>

#include <array>
#include <cmath>
#include <istream>
#include <limits>
#include <ostream>
#include <type_traits>

namespace jb_storage::utility
{

	template < typename T >
	auto Serialize(T val, std::ostream& os) -> std::enable_if_t<std::is_integral_v<T> && sizeof(T) == 1>
	{ os.put(val); }

	template < typename T >
	std::enable_if_t<std::is_integral_v<T> && sizeof(T) == 1, T> Deserialize(std::istream& is)
	{ return is.get();}

	template < typename T >
	auto Serialize(T val, std::ostream& os) -> std::enable_if_t<std::is_integral_v<T> && sizeof(T) != 1>
	{
		std::array<char, sizeof(T)> buffer;
		for (auto& ch : buffer)
		{
			ch = val >> 8 * (sizeof(T) - 1);
			val <<= 8;
		}
		os.write(buffer.data(), buffer.size());
	}

	template < typename T >
	std::enable_if_t<std::is_integral_v<T> && sizeof(T) != 1, T> Deserialize(std::istream& is)
	{
		T val{ };

		std::array<char, sizeof(T)> buffer;
		is.read(buffer.data(), buffer.size());
		for (const auto& ch : buffer)
			val = (val << 8) | (ch & 0xFF) ;

		return val;
	}

	template < typename T >
	auto Serialize(T val, std::ostream& os) -> std::enable_if_t<std::is_same_v<T, float> || std::is_same_v<T, double>>
	{
		int exponent;
		const T mantissa{ std::frexp(val, &exponent) };

		using MantissaCoverageType = std::conditional_t<std::is_same_v<T, float>, int32_t, int64_t>;

		Serialize(static_cast<MantissaCoverageType>(mantissa * std::numeric_limits<MantissaCoverageType>::max()), os);
		Serialize(static_cast<int32_t>(exponent), os);
	}

	template < typename T >
	std::enable_if_t<std::is_same_v<T, float> || std::is_same_v<T, double>, T> Deserialize(std::istream& is)
	{
		using MantissaCoverageType = std::conditional_t<std::is_same_v<T, float>, int32_t, int64_t>;

		const auto mantissa{ Deserialize<MantissaCoverageType>(is) };
		const auto exponent{ Deserialize<int32_t>(is) };

		return ldexp(static_cast<T>(mantissa) / std::numeric_limits<MantissaCoverageType>::max(), exponent);
	}

	template < typename T >
	auto Serialize(const T& blob, std::ostream& os) -> std::enable_if_t<std::is_same_v<T, Blob> || std::is_same_v<T, std::string>>
	{
		Serialize<uint64_t>(blob.size(), os);
		os.write(reinterpret_cast<const char*>(blob.data()), blob.size());
	}

	template < typename T >
	std::enable_if_t<std::is_same_v<T, Blob> || std::is_same_v<T, std::string>, T> Deserialize(std::istream& is)
	{
		const auto size{ Deserialize<uint64_t>(is) };
		T blob(size, 0);
		is.read(reinterpret_cast<char*>(blob.data()), size);
		return blob;
	}

	void Serialize(const Value& val, std::ostream& os);

	template < typename T >
	std::enable_if_t<std::is_same<T, Value>::value, T> Deserialize(std::istream& is);

}

#endif

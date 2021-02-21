#pragma once
namespace boost_extension
{
	struct ToVector {};
	ToVector to_vec = ToVector();

	template<typename Range, typename T = typename Range::value_type>
	std::vector<T> operator | (const Range& range, ToVector)
	{
		return std::vector<T>(std::begin(range), std::end(range));
	}
}

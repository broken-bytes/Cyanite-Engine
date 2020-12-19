#pragma once
#include "pch.hxx"

namespace Cyanite::MathKit::Types {
	template<
		typename T,
		typename = typename std::enable_if<std::is_arithmetic_v<T>>::type
	>
	class Range {
	public:
		[[nodiscard]] auto Min() const {
			return _lower;
		}

		[[nodiscard]] auto Max() const {
			return _upper;
		}

	private:
		T _lower;
		T _upper;
	};
}

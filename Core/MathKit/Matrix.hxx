#pragma once

#include "pch.hxx"

namespace Cyanite::MathKit::Types {
	template<
		typename T,
		uint8_t N,
		uint8_t M,
		typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type
	>
	class Matrix {
	public:
		auto operator[](uint8_t id)->T {
			return _data[id];
		}
	private:
		std::array<std::array<T, M>, N> _data;
	};
}

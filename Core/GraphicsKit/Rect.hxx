#pragma once

#include "pch.hxx"

namespace Cyanite::GraphicsKit::Components {
	template< 
		typename T,
		typename = typename std::enable_if<std::is_arithmetic_v<T>>::type
	>
	struct Rect {
		T Left;
		T Right;
		T Top;
		T Bottom;

		auto Width() {
			return Right - Left;
		}

		auto Height() {
			return Top - Bottom;
		}
	};
}

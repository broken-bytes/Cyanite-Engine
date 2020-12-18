#pragma once

#include "pch.hxx"

template<typename T,
	typename = typename std::enable_if<std::is_arithmetic_v<T>>::type
>
struct Color {
	T R;
	T G;
	T B;
	T A;
};
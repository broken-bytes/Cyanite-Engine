#pragma once
#include "pch.hxx"

namespace Cyanite::GraphicsKit::Types {
	enum class DisplayMode {
		Windowed,
		WindowedBorderless,
		Fullscreen,
		ExclusiveFullscreen
	};

	struct Vertex {
		MathKit::Types::Vector3<float> Position;
		MathKit::Types::Vector2<float> UV;
	};
}

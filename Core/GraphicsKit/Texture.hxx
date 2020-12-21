#pragma once

#include "pch.hxx"

#include "../MathKit/Vector.hxx"

namespace Cyanite::GraphicsKit::Components {
	struct Texture {
		uint32_t Width;
		uint32_t Height;
		uint32_t MipMaps;
		DXGI_FORMAT Format;
		Cyanite::MathKit::Types::Vector2<float> Offset;
		float Rotation;
		std::vector<uint8_t> Data;
	};
}

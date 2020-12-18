#pragma once
#include "pch.hxx"

#include "Color.hxx"
#include "Transform.hxx"
#include "Types.hxx"
#include "../MathKit/Vector.hxx"

namespace Cyanite::GraphicsKit::Components {

	enum LightType {
		Point,
		Directional,
		Spot
	};
	
	struct Light {
		Transform Transform;
		LightType Type;
		Color<float> Color;
		float Falloff;
	};
}



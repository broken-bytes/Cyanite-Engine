#pragma once
#include "pch.hxx"

#include "Light.hxx"

namespace Cyanite::GraphicsKit::Components {
	struct Scene {
		std::vector<Components::Light> Lights;
	};
}

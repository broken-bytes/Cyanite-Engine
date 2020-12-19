#pragma once

#include "pch.hxx"


#include "../MathKit/Range.hxx"
#include "../MathKit/Vector.hxx"


namespace Cyanite::GraphicsKit {
	class Camera {
	public:
		enum class Space {
			Screen,
			Local
		};

		auto Ortographic() const -> bool;
		auto Position() const -> MathKit::Types::Vector3<float>;

	private:
		float _fov;   // Vertical field of view.
		float _aspectRatio; // Aspect ratio
		MathKit::Types::Range<float> _clippingPlane;
	};
}

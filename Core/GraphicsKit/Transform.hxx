#pragma once

#include "pch.hxx"


#include "../MathKit/Quaternion.hxx"
#include "../MathKit/Vector.hxx"

namespace Cyanite::GraphicsKit::Components {

	struct Transform {
	public:
		[[nodiscard]]
		auto Position() const -> Cyanite::MathKit::Types::Vector3<float> {
			return _position;
		}

		[[nodiscard]]
		auto Rotation() const -> Cyanite::MathKit::Types::Quaternion {
			return _rotation;
		}

		[[nodiscard]]
		auto Euler() const -> Cyanite::MathKit::Types::Vector3<float> {
			return _rotation.Vectorized();
		}

		[[nodiscard]]
		auto Scale() const -> Cyanite::MathKit::Types::Vector3<float> {
			return _scale;
		}

		auto Move(Cyanite::MathKit::Types::Vector3<float> translation) {
			this->_position += translation;
		}

		auto Rotate(Cyanite::MathKit::Types::Quaternion rotation) {

		}

		auto Rotate(Cyanite::MathKit::Types::Vector3<float> vector) {

		}

	private:
		Cyanite::MathKit::Types::Vector3<float> _position;
		Cyanite::MathKit::Types::Quaternion _rotation;
		Cyanite::MathKit::Types::Vector3<float> _scale;
	};
}

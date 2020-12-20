#pragma once

#include "pch.hxx"
#include "Vector.hxx"

namespace Cyanite::MathKit::Types {
		struct Quaternion {
		float X;
		float Y;
		float Z;
		float W;

		Quaternion(float X, float Y, float Z, float W) {
			this->X = X;
			this->Y = Y;
			this->Z = Z;
			this->W = W;
		}

		template<typename T,
			typename = typename std::enable_if<std::is_arithmetic_v<T>>::value
		>
			auto operator*(T value) -> Quaternion {
			return Quaternion{
				this->X * value,
				this->Y * value,
				this->Z = value,
				this->W * value
			};
		}

		auto operator==(const Quaternion& rhs) -> bool {
			auto test = Quaternion{0,0,0,0};
			return (this->X == rhs.X && this->Y == rhs.Y && this->Z == rhs.Z && this->W == rhs.W);
		}

		auto operator!=(const Quaternion& rhs) -> bool {
			return !(*this == rhs);
		}

		auto Magnitude() const -> float {
			return 0;
		}

			// TODO - Add Return value
		auto Vectorized() const->Vector3<float> {
			return { 0,0,0 };
		}
	};

}

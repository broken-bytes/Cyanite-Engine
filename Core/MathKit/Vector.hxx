#pragma once

#include "pch.hxx"

//#include <crc/CRC.h>

namespace Cyanite::MathKit::Types {
	struct IVector {
		[[nodiscard]] virtual auto Magnitude() const -> float = 0;
	};

	template<typename T,
		typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
		struct Vector2 : public IVector {
		T X;
		T Y;

		Vector2(T X, T Y) {
			this->X = X;
			this->Y = Y;
		}

		template<typename U,
			typename = typename std::enable_if<std::is_arithmetic_v<U>>::value
		>
		auto operator*(U value) -> Vector2 {
			return Vector2{
				this->X *= value,
				this->Y *= value
			};
		}

		template<typename U,
			typename = typename std::enable_if<std::is_arithmetic_v<U>>::value
		>
			auto operator+=(U value) -> Vector2 {
			return Vector2{
				this->X += value,
				this->Y += value
			};
		}
				
		auto operator==(const Vector2& rhs) -> bool {
			return this->X == rhs.X && this->Y == rhs.Y;
		}

		auto operator!=(const Vector2& rhs) -> bool {
			return !(this == rhs);
		}

		auto Magnitude() const -> float override {
			return static_cast<float>(std::sqrt((X * X) + (Y * Y)));
		}
	};

	template<typename T,
		typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
		struct Vector3 : IVector {
		T X;
		T Y;
		T Z;

		Vector3(T X, T Y, T Z) {
			this->X = X;
			this->Y = Y;
			this->Z = Z;
		}

		template<typename U,
			typename = typename std::enable_if<std::is_arithmetic_v<U>>::value
		>
			auto operator*(U value) -> Vector3 {
			return Vector3{
				this->X *= value,
				this->Y *= value,
				this->Z *= value
			};
		}

		template<typename U,
			typename = typename std::enable_if<std::is_arithmetic_v<U>>::value
		>
			auto operator+(U value) -> Vector3 {
			return Vector3{
				this->X += value,
				this->Y += value,
				this->Z += value
			};
		}

		auto operator+=(Vector2<T> value) -> Vector3 {
			return Vector3{
				this->X += value.X,
				this->Y += value.Y,
				this->Z
			};
		}

		auto operator+=(Vector3<T> value) -> Vector3 {
			return Vector3{
				this->X += value.X,
				this->Y += value.Y,
				this->Z += value.Z
			};
		}
		
		auto operator==(const Vector3& rhs) -> bool {
			return this->X == rhs.X && this->Y == rhs.Y && this->Z == rhs.Z;
		}

		auto operator!=(const Vector3& rhs) -> bool {
			return !(this == rhs);
		}

		auto Magnitude() const -> float override {
			return std::sqrt((X * X) + (Y * Y) + (Z * Z));
		}
	};

	template<typename T,
		typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
		struct Vector4 : IVector {
		T X;
		T Y;
		T Z;
		T W;

		Vector4(T X, T Y, T Z, T W) {
			this->X = X;
			this->Y = Y;
			this->Z = Z;
			this->W = W;
		}

		template<typename U,
			typename = typename std::enable_if<std::is_arithmetic_v<U>>::value
		>
			auto operator*(U value) -> Vector4 {
			return Vector4{
				this->X *= value,
				this->Y *= value,
				this->Z *= value,
				this->W *= value
			};
		}

		template<typename U,
			typename = typename std::enable_if<std::is_arithmetic_v<U>>::value
		>
			auto operator+(U value) -> Vector4 {
			return Vector4{
				this->X += value,
				this->Y += value,
				this->Z += value,
				this->W += value
			};
		}

		auto operator+=(Vector2<T> value) -> Vector4 {
			return Vector3{
				this->X += value,
				this->Y += value,
				this->Z,
				this->W
			};
		}

		auto operator+=(Vector3<T> value) -> Vector4 {
			return Vector3{
				this->X += value.X,
				this->Y += value.Y,
				this->Z += value.Z,
				this->W
			};
		}

		auto operator+=(Vector4<T> value) -> Vector4 {
			return Vector3{
				this->X += value.X,
				this->Y += value.Y,
				this->Z += value.Z,
				this->W += value.W
			};
		}
			
		auto operator==(const Vector4& rhs) -> bool {
			return this->X == rhs.X && this->Y == rhs.Y && this->Z == rhs.Z && this->W == rhs.W;
		}

		auto operator!=(const Vector4& rhs) -> bool {
			return !(this == rhs);
		}

		auto Magnitude() const -> float override {
			return std::sqrt((X * X) + (Y * Y) + (Z * Z) + (W * W));
		}
	};
}

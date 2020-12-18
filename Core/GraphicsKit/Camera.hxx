#pragma once

#include "pch.hxx"

#include "../MathKit/Vector.hxx"

struct Camera {
	bool Orthographic;
	Cyanite::MathKit::Types::Vector3<float> Position;
};

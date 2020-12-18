#pragma once
#include "pch.hxx"

namespace Cyanite::GraphicsKit::DeviceHandler {
	auto QueryAdapters()->winrt::com_ptr<IDXGIAdapter4>;
}


#pragma once
#include "pch.hxx"

namespace Cyanide::GraphicsKit::DeviceHandler {
	auto QueryAdapters()->winrt::com_ptr<IDXGIAdapter4>;
}


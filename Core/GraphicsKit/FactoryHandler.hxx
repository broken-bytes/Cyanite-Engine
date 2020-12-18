#pragma once

#include "pch.hxx"

namespace Cyanite::GraphicsKit::FactoryHandler {
	/// <summary>
	/// Creates or returns the factory
	/// </summary>
	/// <returns>com_ptr to factory</returns>
	auto CreateFactory()->winrt::com_ptr<IDXGIFactory7>;
	auto DisposeFactory(winrt::com_ptr<IDXGIFactory7> factory);
}

#pragma once
#include "pch.hxx"

namespace Cyanite::GraphicsKit::SwapChainHandler {
	constexpr uint8_t BufferCount = 2;

	auto CreateSwapChain(winrt::com_ptr<ID3D12CommandQueue> queue);
	auto CreateSwapChainFor(
		HWND handle,
		winrt::com_ptr<ID3D12CommandQueue> queue
	) -> winrt::com_ptr<IDXGISwapChain4>;
	auto RemoveSwapChain(winrt::com_ptr<IDXGISwapChain4> sChain);
	auto RemoveSwapChainFrom(HWND handle);
}


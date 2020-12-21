#pragma once
#include "pch.hxx"

namespace Cyanite::GraphicsKit::SwapChainHandler {
	constexpr uint8_t BufferCount = 2;

	[[nodiscard]]
	auto CreateSwapChain(
		winrt::com_ptr<ID3D12CommandQueue> queue
	)->winrt::com_ptr<IDXGISwapChain4>;
	[[nodiscard]]
	auto CreateSwapChainFor(
		HWND handle,
		winrt::com_ptr<ID3D12CommandQueue> queue
	) -> winrt::com_ptr<IDXGISwapChain4>;
	[[nodiscard]]
	auto RemoveSwapChain(winrt::com_ptr<IDXGISwapChain4> sChain);
	[[nodiscard]]
	auto RemoveSwapChainFrom(HWND handle);
}


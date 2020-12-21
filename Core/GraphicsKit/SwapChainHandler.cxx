#include "pch.hxx"
#include "SwapChainHandler.hxx"

#include "FactoryHandler.hxx"
#include "Rect.hxx"
#include "TypeConverter.hxx"


namespace Cyanite::GraphicsKit::SwapChainHandler {
	auto CreateSwapChain(
		winrt::com_ptr<ID3D12CommandQueue> queue
	) -> winrt::com_ptr<IDXGISwapChain4> {
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Flags = 0;
		swapChainDesc.BufferCount = Frames;

		winrt::com_ptr<IDXGISwapChain> swapChain;

		auto factory = FactoryHandler::CreateFactory();
		winrt::check_hresult(
			factory->CreateSwapChain(
				queue.get(),
				&swapChainDesc,
				swapChain.put()
			)
		);

		return Tools::UseAs<IDXGISwapChain4>(swapChain);
	}

	auto RemoveSwapChain(winrt::com_ptr<IDXGISwapChain4> sChain) {}
	auto RemoveSwapChainFrom(HWND handle) {}
	auto CreateSwapChainFor(HWND handle, winrt::com_ptr<ID3D12CommandQueue> queue) ->
		winrt::com_ptr<IDXGISwapChain4> {

		RECT r;
		GetWindowRect(handle, &r);

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = r.right - r.left;
		swapChainDesc.Height = r.bottom - r.top;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDesc.Flags = 0;
		swapChainDesc.BufferCount = Frames;

		winrt::com_ptr<IDXGISwapChain1> swapChain;
		
		auto factory = FactoryHandler::CreateFactory();
		winrt::check_hresult(
			factory->CreateSwapChainForHwnd(
				queue.get(),
				handle,
				&swapChainDesc,
				nullptr,
				nullptr,
				swapChain.put()
			)
		);

		return Tools::UseAs<IDXGISwapChain4>(swapChain);
	}
}

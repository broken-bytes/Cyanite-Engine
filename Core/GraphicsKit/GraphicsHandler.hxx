#pragma once
#include <cstdint>
#include "pch.hxx"

#include "Frame.hxx"
#include "Gpu.hxx"
#include "Texture.hxx"
#include "../EventKit/EventRelay.hxx"

namespace Cyanite::GraphicsKit {
	class GraphicsHandler {
	public:
		explicit GraphicsHandler(HWND window);
		~GraphicsHandler();
		GraphicsHandler(const GraphicsHandler&) = delete;
		GraphicsHandler(GraphicsHandler&&) = delete;
		GraphicsHandler& operator=(GraphicsHandler&) = delete;
		GraphicsHandler& operator=(GraphicsHandler&&) = delete;

		auto Initialize() -> void;
		auto Deinitialize() -> void;
		auto Update() -> void;
		auto Render() -> void;
		auto Resize(uint32_t width, uint32_t height) -> void;

	private:

        // Pipeline objects.
        CD3DX12_VIEWPORT _viewport;
        CD3DX12_RECT _scissorRect;
        winrt::com_ptr<IDXGISwapChain4> _swapChain;
        winrt::com_ptr<ID3D12Device> _device;
        winrt::com_ptr<ID3D12Resource> _renderTargets[Frames];
        winrt::com_ptr<ID3D12Resource> _depthStencil;
        winrt::com_ptr<ID3D12CommandAllocator> _commandAllocator;
        winrt::com_ptr<ID3D12RootSignature> _rootSignature;
        winrt::com_ptr<ID3D12DescriptorHeap> _rtvHeap;
        uint64_t m_rtvDescriptorSize;
        winrt::com_ptr<ID3D12DescriptorHeap> _dsvHeap;
        winrt::com_ptr<ID3D12DescriptorHeap> _cbvSrvHeap;
        winrt::com_ptr<ID3D12DescriptorHeap> _samplerHeap;
        winrt::com_ptr<ID3D12PipelineState> _pipelineState;
		
        uint64_t _frameIndex;
        HANDLE _fenceEvent;
        winrt::com_ptr<ID3D12Fence> _fence;
        uint64_t _fenceValue;
	};
}

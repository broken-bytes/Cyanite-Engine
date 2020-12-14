#pragma once
#include <cstdint>
#include "pch.hxx"


namespace Cyanide::GraphicsKit {
	// Double buffered
	constexpr uint8_t FrameCount = 2;
	constexpr D3D_FEATURE_LEVEL MIN_D3D_LVL = D3D_FEATURE_LEVEL_11_1;
	
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

		struct Vertex
		{
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT4 color;
		};


		// Viewport data
		uint32_t _width;
		uint32_t _height;
		float _aspectRatio;
		bool _useWarpDevice;
		HWND _window;

		// Pipeline objects.
		D3D12_VIEWPORT _viewport;
		D3D12_RECT _scissorRect;
		winrt::com_ptr<IDXGISwapChain3> _swapChain;
		winrt::com_ptr<ID3D12Device> _device;
		winrt::com_ptr<ID3D12Resource> _renderTargets[FrameCount];
		winrt::com_ptr<ID3D12CommandAllocator> _commandAllocator;
		winrt::com_ptr<ID3D12CommandQueue> _commandQueue;
		winrt::com_ptr<ID3D12RootSignature> _rootSignature;
		winrt::com_ptr<ID3D12DescriptorHeap> _rtvHeap;
		winrt::com_ptr<ID3D12PipelineState> _pipelineState;
		winrt::com_ptr<ID3D12GraphicsCommandList> _commandList;
		UINT _rtvDescriptorSize;

		// App resources.
		winrt::com_ptr<ID3D12Resource> _vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;

		// Synchronization objects.
		UINT _frameIndex;
		HANDLE _fenceEvent;
		winrt::com_ptr<ID3D12Fence> _fence;
		UINT64 _fenceValue;

		auto LoadPipeline() -> void;
		auto LoadAssets() -> void;
		auto PopulateCommandList() -> void;
		auto WaitForPreviousFrame() -> void;
	};
}
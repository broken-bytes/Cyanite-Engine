#pragma once
#include <cstdint>
#include "pch.hxx"

#include "Frame.hxx"
#include "Gpu.hxx"


namespace Cyanite::GraphicsKit {
	namespace Components {
		struct Texture;
	}

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
		Types::DisplayMode _displayMode;

		// Pipeline objects.
		CD3DX12_VIEWPORT _viewport;
		CD3DX12_RECT _scissorRect;
		winrt::com_ptr<IDXGIFactory7> _factory;
		winrt::com_ptr<IDXGISwapChain4> _swapChain;
		std::unique_ptr<Gpu> _device;


		std::array<winrt::com_ptr<ID3D12Resource>, Frames> _renderTargets;
		std::array<winrt::com_ptr<ID3D12CommandAllocator>, Frames> _allocs;
		std::array<winrt::com_ptr<ID3D12CommandQueue>, Frames> _queues;
		std::array<winrt::com_ptr<ID3D12GraphicsCommandList>, Frames> _lists;
		
		std::array<winrt::com_ptr<ID3D12Fence1>, Frames> _fences;
		std::array<uint64_t, Frames> _fenceValues;
		uint64_t _frameIndex;
		uint64_t _rtvDescriptorSize;
		HANDLE _fenceEvent;
		
		winrt::com_ptr<ID3D12DescriptorHeap> _rtvHeap;
		
		winrt::com_ptr<ID3D12RootSignature> _rootSignature;
		winrt::com_ptr<ID3D12PipelineState> _pipelineState;
		winrt::com_ptr<ID3D12PipelineState> _pipelineStateShadowMap;

		// App resources.
		D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW _indexBufferView;
		winrt::com_ptr<ID3D12Resource> _textures[1];
		winrt::com_ptr<ID3D12Resource> _textureUploads[1];
		winrt::com_ptr<ID3D12Resource> _indexBuffer;
		winrt::com_ptr<ID3D12Resource> _indexBufferUpload;
		winrt::com_ptr<ID3D12Resource> _vertexBuffer;
		winrt::com_ptr<ID3D12Resource> _vertexBufferUpload;
		winrt::com_ptr<ID3D12Resource> _depthStencil;

		auto SetDebugMode() -> void;
		auto Flush(
			winrt::com_ptr<ID3D12CommandQueue> commandQueue,
			winrt::com_ptr<ID3D12Fence> fence,
			uint64_t& fenceValue,
			HANDLE fenceEvent
		) -> void;

		auto FrameStart() -> void;
		auto FrameMid() const -> void;
		auto FrameEnd() -> void;
		auto UpdatePipeline() -> void;

		
		auto SetDefaultPipeline(winrt::com_ptr<ID3D12GraphicsCommandList> commands) -> void;
		auto LoadPipeline() -> void;
		auto LoadAssets() -> void;
		auto CreateRootSignature() -> void;
		auto CreatePipelineState() -> void;
		auto CreateRenderTargetViews() -> void;
		auto CreateDepthStencil() -> void;
		auto CreateVertexBuffer() -> void;
		auto CopyToVertexBuffer(
			winrt::com_ptr<ID3D12GraphicsCommandList> list,
			D3D12_SUBRESOURCE_DATA vertexData
		) -> void;
		auto CreateIndexBuffer() -> void;
		auto CopyToIndexBuffer(
			winrt::com_ptr<ID3D12GraphicsCommandList> list,
			D3D12_SUBRESOURCE_DATA indexData) -> void;
		auto CreateShaderResource() -> void;
		auto UploadTextures(
			std::vector<Components::Texture> textures 
		) -> void;
		auto PopulateCommandList() -> void;
		auto AwaitFrameCompletion() -> void;

		auto Worker(uint8_t id) -> void;
	};
}

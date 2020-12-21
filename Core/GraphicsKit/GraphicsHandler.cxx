#include "pch.hxx"
#include "GraphicsHandler.hxx"
#include "TypeConverter.hxx"

namespace Cyanite::GraphicsKit {
	GraphicsHandler::GraphicsHandler(HWND window) {
		SetDebugMode();
		_device = std::make_unique<Gpu>(window);
		_device->GetError();
		auto t = "";
	}
	GraphicsHandler::~GraphicsHandler() {}
	auto GraphicsHandler::Initialize() -> void {
		_device->GetError();
		for (uint8_t x = 0; x < Frames; x++) {
			_allocs[x] = _device->CreateCommandAllocator();
		}
		_device->GetError();
		_list = _device->CreateCommandList(_allocs[0]);
		_list->Reset(_allocs[0].get(), nullptr);
	}
	auto GraphicsHandler::Deinitialize() -> void {
		// wait for the gpu to finish all frames
		for (int x = 0; x < Frames; ++x)
		{
			AwaitFrameCompletion();
		}

		// get swapchain out of full screen before exiting
		BOOL fs = false;
		if (_swapChain->GetFullscreenState(&fs, nullptr))
			_swapChain->SetFullscreenState(false, nullptr);

		delete _device.release();
		_swapChain = nullptr;
		_list = nullptr;

		_rtvHeap = nullptr;

		for (int x = 0; x < Frames; ++x)
		{
			_renderTargets[x] = nullptr;
			_allocs[x] = nullptr;
		};
	}

	auto GraphicsHandler::Update() -> void {}
	auto GraphicsHandler::Render() -> void {
		UpdatePipeline();
		_device->ExecuteDirect({ _list });
	}
	auto GraphicsHandler::Resize(uint32_t width, uint32_t height) -> void {}
	auto GraphicsHandler::SetDebugMode() -> void {
#if defined(_DEBUG)
		// Always enable the debug layer before doing anything DX12 related
		// so all possible errors generated while creating DX12 objects
		// are caught by the debug layer.
		winrt::com_ptr<ID3D12Debug> debugInterface;
		winrt::check_hresult(
			D3D12GetDebugInterface(
				IID_PPV_ARGS(debugInterface.put()
				)
			)
		);
		debugInterface->EnableDebugLayer();
#endif
	}

	auto GraphicsHandler::Flush(winrt::com_ptr<ID3D12CommandQueue> commandQueue, winrt::com_ptr<ID3D12Fence> fence,
		uint64_t& fenceValue, HANDLE fenceEvent) -> void {}

	auto GraphicsHandler::FrameStart() -> void {}
	auto GraphicsHandler::FrameMid() const -> void {}
	auto GraphicsHandler::FrameEnd() -> void {}
	auto GraphicsHandler::UpdatePipeline() -> void {
		_device->Update(_list);
	}
	auto GraphicsHandler::SetDefaultPipeline(winrt::com_ptr<ID3D12GraphicsCommandList> commands) -> void {}
	auto GraphicsHandler::LoadPipeline() -> void {}
	auto GraphicsHandler::LoadAssets() -> void {}
	auto GraphicsHandler::CreateRootSignature() -> void {}
	auto GraphicsHandler::CreatePipelineState() -> void {}
	auto GraphicsHandler::CreateRenderTargetViews() -> void {}
	auto GraphicsHandler::CreateDepthStencil() -> void {}
	auto GraphicsHandler::CreateVertexBuffer() -> void {}

	auto GraphicsHandler::CopyToVertexBuffer(winrt::com_ptr<ID3D12GraphicsCommandList> list,
		D3D12_SUBRESOURCE_DATA vertexData) -> void {}

	auto GraphicsHandler::CreateIndexBuffer() -> void {}

	auto GraphicsHandler::CopyToIndexBuffer(winrt::com_ptr<ID3D12GraphicsCommandList> list,
		D3D12_SUBRESOURCE_DATA indexData) -> void {}

	auto GraphicsHandler::CreateShaderResource() -> void {}
	auto GraphicsHandler::UploadTextures(std::vector<Components::Texture> textures) -> void {}
	auto GraphicsHandler::PopulateCommandList() -> void {}
	auto GraphicsHandler::AwaitFrameCompletion() -> void {
		// swap the current rtv buffer index so we draw on the correct buffer
		_device->Wait();
	}
	auto GraphicsHandler::Worker(uint8_t id) -> void {}
}

#include "pch.hxx"
#include "GraphicsHandler.hxx"

namespace Cyanite::GraphicsKit {
	GraphicsHandler::GraphicsHandler(HWND window) {
		_device = std::make_unique<Gpu>();
	}
	GraphicsHandler::~GraphicsHandler() {}
	auto GraphicsHandler::Initialize() -> void {
		_lists = {};
		_allocs = {};
		_queue = _device->CreateCommandQueue();

		for (uint8_t x = 0; x < Frames; x++) {
			_allocs[x] = _device->CreateCommandAllocator();
			_fences[x] = _device->CreateFence();
			_fenceValues[x] = 0;
		}
		_lists[0] = _device->CreateCommandList(_allocs[0]);
		_lists[1] = _device->CreateCommandList(_allocs[0]);
		_fenceEvent = CreateEvent(
			nullptr,
			false,
			false,
			nullptr
		);

	}
	auto GraphicsHandler::Deinitialize() -> void {
		// wait for the gpu to finish all frames
		for (int x = 0; x < Frames; ++x)
		{
			_frameIndex = x;
			AwaitFrameCompletion();
		}

		// get swapchain out of full screen before exiting
		BOOL fs = false;
		if (_swapChain->GetFullscreenState(&fs, nullptr))
			_swapChain->SetFullscreenState(false, nullptr);


		_swapChain = nullptr;
		_queue = nullptr;
		for (int x = 0; x < _lists.size(); x++) {
			_lists[x] = nullptr;
		}

		_rtvHeap = nullptr;

		for (int x = 0; x < Frames; ++x)
		{
			_renderTargets[x] = nullptr;
			_allocs[x] = nullptr;
			_fences[x] = nullptr;
		};
	}

	auto GraphicsHandler::Update() -> void {}
	auto GraphicsHandler::Render() -> void {
		UpdatePipeline();

		std::vector<ID3D12CommandList*> lists(_lists.size());
		for (auto list : _lists) {
			lists.emplace_back(list);
		}

		_queue->ExecuteCommandLists(
			_lists.size(),
			lists.data()
		);

		winrt::check_hresult(
			_queue->Signal(
				_fences[_frameIndex].get(),
				_fenceValues[_frameIndex]
			)
		);

		winrt::check_hresult(_swapChain->Present(0, 0));
	}
	auto GraphicsHandler::Resize(uint32_t width, uint32_t height) -> void {}
	auto GraphicsHandler::SetDebugMode() -> void {}

	auto GraphicsHandler::Flush(winrt::com_ptr<ID3D12CommandQueue> commandQueue, winrt::com_ptr<ID3D12Fence> fence,
		uint64_t& fenceValue, HANDLE fenceEvent) -> void {}

	auto GraphicsHandler::FrameStart() -> void {}
	auto GraphicsHandler::FrameMid() const -> void {}
	auto GraphicsHandler::FrameEnd() -> void {}
	auto GraphicsHandler::UpdatePipeline() -> void {
		AwaitFrameCompletion();

		winrt::check_hresult(_allocs[_frameIndex]->Reset());
		winrt::check_hresult(
			_lists[0]->Reset(
				_allocs[_frameIndex].get(),
				nullptr
			)
		);

		_lists[0]->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				_renderTargets[_frameIndex].get(),
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			)
		);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
			_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
			_frameIndex,
			_rtvDescriptorSize
		);
		_lists[0]->OMSetRenderTargets(
			1,
			&rtvHandle,
			false,
			nullptr
		);

		float color[] = { 0,0,0,0 };
		_lists[0]->ClearRenderTargetView(
			rtvHandle,
			color,
			0,
			nullptr
		);

		_lists[0]->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				_renderTargets[_frameIndex].get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT
			)
		);

		_lists[0]->Close();
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
		_frameIndex = _swapChain->GetCurrentBackBufferIndex();

		if (_fences[_frameIndex]->GetCompletedValue() <
			_fenceValues[_frameIndex])
		{
			winrt::check_hresult(
				_fences[_frameIndex]->SetEventOnCompletion(
					_fenceValues[_frameIndex],
					_fenceEvent
				)
			);

			WaitForSingleObject(_fenceEvent, INFINITE);
		}
		_fenceValues[_frameIndex]++;
	}
	auto GraphicsHandler::Worker(uint8_t id) -> void {}
}

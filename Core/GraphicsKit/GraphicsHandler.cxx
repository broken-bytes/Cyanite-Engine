#include "pch.hxx"
#include "GraphicsHandler.hxx"
#include "TypeConverter.hxx"
#include "../../Libs/ImGUI/imgui.h"
#include "../../Libs/ImGUI/imgui_impl_dx12.h"
#include "../../Libs/ImGUI/imgui_impl_win32.h"

auto open = true;

namespace Cyanite::GraphicsKit {
	GraphicsHandler::GraphicsHandler(HWND window) {
		SetDebugMode();
		_device = std::make_unique<Gpu>(window);
		_relay = std::make_unique<EventKit::EventRelay>(
			[this](EventKit::IEvent event) {
				this->EventHandler(event);
			}
		);
	}
	GraphicsHandler::~GraphicsHandler() {}
	auto GraphicsHandler::Initialize() -> void {
		_list = _device->CreateCommandList(
			_device->DirectAlloc(
				0,
				0
			)
		);
		_list->Close();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsLight();
		ImGui_ImplWin32_Init(_window);
		ImGui_ImplDX12_Init(
			_device->Device().get(),
			Frames,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			_device->SrvHeap().get(),
			_device->SrvHeap().get()->GetCPUDescriptorHandleForHeapStart(),
			_device->SrvHeap().get()->GetGPUDescriptorHandleForHeapStart()
		);
	}
	auto GraphicsHandler::Deinitialize() -> void {
		// wait for the gpu to finish all frames
		for (int x = 0; x < Frames; ++x)
		{
			AwaitFrameCompletion();
		}

		delete _device.release();
		_swapChain = nullptr;
		_list = nullptr;

		_rtvHeap = nullptr;

		for (int x = 0; x < Frames; ++x)
		{
			_renderTargets[x] = nullptr;
		};
	}

	auto GraphicsHandler::Update() -> void {}
	auto GraphicsHandler::Render() -> void {
		float color[] = { 0,0,0,0 };
		UpdatePipeline();

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Test");
		ImGui::Text("Test");
		ImGui::End();
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(
			ImGui::GetDrawData(),
			_list.get()
		);
		ImGui::EndFrame();

		auto imgui = ImGui::GetDrawData();
		ImGui_ImplDX12_RenderDrawData(imgui, _list.get());

		_device->ExecuteDirect({ _list });
		_device->Draw();
	}
	auto GraphicsHandler::Resize(uint32_t width, uint32_t height) -> void {
		ImGui_ImplDX12_InvalidateDeviceObjects();
		ImGui_ImplDX12_CreateDeviceObjects();
		//_device->Resize(width, height);
	}
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
	auto GraphicsHandler::EventHandler(EventKit::IEvent event) -> void {
		if (event.Type == EventKit::EventType::WindowResize) {
			MathKit::Types::Vector2<uint32_t> dim{ 0,0 };
			dim = *static_cast<MathKit::Types::Vector2<uint32_t>*>(event.Data);
			Resize(dim.X, dim.Y);
		}
	}
}

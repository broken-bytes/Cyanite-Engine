#include "pch.hxx"
#include "Gpu.hxx"

#include "DeviceHandler.hxx"
#include "FactoryHandler.hxx"
#include "SwapChainHandler.hxx"
namespace Cyanite::GraphicsKit {
	Gpu::Gpu(HWND window) {
		auto adapter = DeviceHandler::QueryAdapters();
		_window = window;
		winrt::check_hresult(D3D12CreateDevice(
			adapter.get(),
			MIN_D3D_LVL,
			IID_PPV_ARGS(_device.put())
		));

		_directQueue = CreateCommandQueue();
		_copyQueue = CreateCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
		_computeQueue = CreateCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);

		for (uint8_t x = 0; x < CommandQueues; x++) {
			_fenceEvent[x] = CreateEvent(
				nullptr,
				false,
				false,
				nullptr
			);
		}
		_swapChain = CreateSwapChain(_window);
		FactoryHandler::CreateFactory()->MakeWindowAssociation(
			_window,
			DXGI_MWA_NO_ALT_ENTER
		);

		_frameIndex = _swapChain->GetCurrentBackBufferIndex();

		_rtvHeap = CreateDescriptorHeap(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			Frames
		);
		
		_rtvSize = _device->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV
		);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
			_rtvHeap->GetCPUDescriptorHandleForHeapStart()
		);

		_srvHeap = CreateDescriptorHeap(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			1,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
		);

		_srvSize = _device->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
		);

		CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(
			_srvHeap->GetCPUDescriptorHandleForHeapStart()
		);

		CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle2(
			_srvHeap->GetGPUDescriptorHandleForHeapStart()
		);
		
		for (int x = 0; x < Frames; x++)
		{
			winrt::check_hresult(
				_swapChain->GetBuffer(
					x,
					IID_PPV_ARGS(_renderTargets[x].put())
				)
			);
			_device->CreateRenderTargetView(
				_renderTargets[x].get(),
				nullptr,
				rtvHandle
			);
			rtvHandle.Offset(1, _rtvSize);
		}

		for (int x = 0; x < Frames; x++)
		{
			winrt::check_hresult(
				_device->CreateFence(
					0,
					D3D12_FENCE_FLAG_NONE,
					IID_PPV_ARGS(_fences[x].put())
				)
			);
			_fenceValues[x] = 0;
		}
		_fenceEvent[0] = CreateEvent(
			nullptr,
			false,
			false,
			nullptr
		);

		for(int x = 0; x < _directAlloc.size(); x++) {
			_directAlloc[x] = CreateCommandAllocator();
		}
	}

	Gpu::~Gpu() {
		_device = nullptr;
		_frameIndex = Frames;
	}

	auto Gpu::Device() -> winrt::com_ptr<ID3D12Device> {
		return _device;
	}

	auto Gpu::FrameIndex() const -> uint64_t {
		return _frameIndex;
	}

	auto Gpu::FenceValue(uint64_t frame) -> uint64_t {
		return _fences[_frameIndex]->GetCompletedValue();
	}

	auto Gpu::CreateSwapChain(std::optional<HWND> handle) -> winrt::com_ptr<IDXGISwapChain4> {
		if (handle.has_value()) {
			return SwapChainHandler::CreateSwapChainFor(
				handle.value(),
				_directQueue
			);
		}
		return SwapChainHandler::CreateSwapChain(_directQueue);
	}

	auto Gpu::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type,
		D3D12_COMMAND_QUEUE_PRIORITY priority) -> winrt::com_ptr<ID3D12CommandAllocator> {
		winrt::com_ptr<ID3D12CommandAllocator> alloc;
		winrt::check_hresult(
			_device->CreateCommandAllocator(
				type, IID_PPV_ARGS(alloc.put())
			)
		);
		return alloc;
	}


	auto Gpu::CreateCommandList(
		winrt::com_ptr<ID3D12CommandAllocator> alloc,
		D3D12_COMMAND_LIST_TYPE type
	)
		-> winrt::com_ptr<ID3D12GraphicsCommandList> {
		winrt::com_ptr<ID3D12GraphicsCommandList6> list;
		winrt::check_hresult(
			_device->CreateCommandList(
				0,
				type,
				alloc.get(),
				nullptr,
				IID_PPV_ARGS(list.put())
			)
		);

		return list;
	}

	auto Gpu::CreateCommandQueue(
		D3D12_COMMAND_LIST_TYPE type,
		D3D12_COMMAND_QUEUE_PRIORITY priority
	) -> winrt::com_ptr<ID3D12CommandQueue> {
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = type;
		desc.Priority = priority;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		winrt::com_ptr<ID3D12CommandQueue> queue;
		winrt::check_hresult(
			_device->CreateCommandQueue(
				&desc,
				IID_PPV_ARGS(queue.put())
			)
		);
		return queue;
	}

	auto Gpu::CreateDescriptorHeap(
		D3D12_DESCRIPTOR_HEAP_TYPE type,
		uint32_t numDescriptors,
		D3D12_DESCRIPTOR_HEAP_FLAGS flags
	) -> winrt::com_ptr<ID3D12DescriptorHeap> {
		D3D12_DESCRIPTOR_HEAP_DESC rtv = {};
		winrt::com_ptr<ID3D12DescriptorHeap> heap;
		rtv.Type = type;
		rtv.NumDescriptors = numDescriptors;
		rtv.Flags = flags;
		winrt::check_hresult(
			_device->CreateDescriptorHeap(
				&rtv,
				IID_PPV_ARGS(heap.put())
			)
		);

		return heap;
	}

	auto Gpu::DrawRtvs() -> void {
		auto rtvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
			_rtvHeap->GetCPUDescriptorHandleForHeapStart()
		);

		for (int x = 0; x < Frames; ++x)
		{
			winrt::com_ptr<ID3D12Resource> backBuffer;
			winrt::check_hresult(
				_swapChain->GetBuffer(x,
					IID_PPV_ARGS(backBuffer.put())
				)
			);

			_device->CreateRenderTargetView(backBuffer.get(), nullptr, rtvHandle);

			_buffers[x] = backBuffer;

			rtvHandle.Offset(rtvDescriptorSize);
		}

	}

	auto Gpu::Draw() -> void {
		winrt::check_hresult(_swapChain->Present(0, 0));
	}

	auto Gpu::Update(winrt::com_ptr<ID3D12GraphicsCommandList> list) -> void {
		Wait();

		winrt::check_hresult(_directAlloc[_frameIndex]->Reset());
		winrt::check_hresult(
			list->Reset(
				_directAlloc[_frameIndex].get(),
				nullptr
			)
		);
		
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			_renderTargets[FrameIndex()].get(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		);

		list->ResourceBarrier(
			1,
			&barrier
		);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
			_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
			FrameIndex(),
			_rtvSize
		);
		list->OMSetRenderTargets(
			1,
			&rtvHandle,
			false,
			nullptr
		);

		list->SetDescriptorHeaps(
			1,
			_srvHeap.put()
		);
		
		float color[] = { 0,0,0,0 };
		list->ClearRenderTargetView(
			rtvHandle,
			color,
			0,
			nullptr
		);


		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			_renderTargets[FrameIndex()].get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		);
		list->ResourceBarrier(
			1,
			&barrier
		);

		winrt::check_hresult(list->Close());
	}

	auto Gpu::Signal(
		winrt::com_ptr<ID3D12CommandQueue> commandQueue,
		winrt::com_ptr<ID3D12Fence> fence
	) -> uint64_t {
		uint64_t fenceValueForSignal = ++_fenceValues[_frameIndex];
		winrt::check_hresult(
			commandQueue->Signal(
				fence.get(),
				fenceValueForSignal)
		);

		return fenceValueForSignal;
	}

	auto Gpu::CreateFence(uint64_t value) -> winrt::com_ptr<ID3D12Fence1> {
		winrt::com_ptr<ID3D12Fence1> fence;

		winrt::check_hresult(
			_device->CreateFence(
				value,
				D3D12_FENCE_FLAG_NONE,
				IID_PPV_ARGS(fence.put())
			)
		);

		return fence;
	}

	auto Gpu::ExecuteDirect(
		std::vector<winrt::com_ptr<ID3D12GraphicsCommandList>> lists
	) -> void {
		ExecuteCommandLists(lists, D3D12_COMMAND_LIST_TYPE_DIRECT);
	}
	auto Gpu::ExecuteCopy(
		std::vector<winrt::com_ptr<ID3D12GraphicsCommandList>> lists
	) -> void {
		ExecuteCommandLists(lists, D3D12_COMMAND_LIST_TYPE_COPY);
	}
	auto Gpu::ExecuteCompute(
		std::vector<winrt::com_ptr<ID3D12GraphicsCommandList>> lists
	) -> void {
		ExecuteCommandLists(lists, D3D12_COMMAND_LIST_TYPE_COMPUTE);
	}
	auto Gpu::ExecuteBundle(
		std::vector<winrt::com_ptr<ID3D12GraphicsCommandList>> lists
	) -> void {
		ExecuteCommandLists(lists, D3D12_COMMAND_LIST_TYPE_BUNDLE);
	}

	auto Gpu::Wait() -> void {
		if (
			_fences[_frameIndex]->GetCompletedValue() <
			_fenceValues[_frameIndex]
			) {
			winrt::check_hresult(
				_fences[_frameIndex]->SetEventOnCompletion(
					_fenceValues[_frameIndex],
					_fenceEvent[0]
				)
			);

			WaitForSingleObject(_fenceEvent[0], INFINITE);
		}
		_fenceValues[_frameIndex]++;
		_frameIndex = _swapChain->GetCurrentBackBufferIndex();
	}

	auto Gpu::GetError() -> void {
		auto reason = _device->GetDeviceRemovedReason();
#if defined(_DEBUG)
		wchar_t outString[100];
		size_t size = 100;
		swprintf_s(outString, size, L"Device removed! DXGI_ERROR code: 0x%X\n", reason);
		OutputDebugStringW(outString);
#endif
	}

	auto Gpu::SrvHeap() -> winrt::com_ptr<ID3D12DescriptorHeap> {
		return _srvHeap;
	}
	auto Gpu::RtvHeap() -> winrt::com_ptr<ID3D12DescriptorHeap> {
		return _rtvHeap;
	}

	auto Gpu::DirectAlloc(uint8_t threadId, uint64_t frameId) -> winrt::com_ptr<ID3D12CommandAllocator> {
		return _directAlloc[frameId];
	}

	auto Gpu::ExecuteCommandLists(
		std::vector<winrt::com_ptr<ID3D12GraphicsCommandList>> lists,
		D3D12_COMMAND_LIST_TYPE type
	) -> void {


		std::vector<ID3D12CommandList*> raw;

		for (int x = 0; x < lists.size(); x++) {
			raw.emplace_back(lists[x].get());
		}
		;

		_directQueue->ExecuteCommandLists(
			lists.size(),
			raw.data()
		);


		winrt::check_hresult(
			_directQueue->Signal(
				_fences[_frameIndex].get(),
				_fenceValues[_frameIndex]
			)
		);
	}
}

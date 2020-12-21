#include "pch.hxx"
#include "Gpu.hxx"

#include "DeviceHandler.hxx"
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
		_bundleQueue = CreateCommandQueue(D3D12_COMMAND_LIST_TYPE_BUNDLE);

		for (uint8_t x = 0; x < Frames; x++) {
			_fences[x] = CreateFence();
			_fenceValues[x] = 0;
		}

		for(uint8_t x = 0; x < CommandQueues; x++) {
			_fenceEvent[x] = CreateEvent(
				nullptr,
				false,
				false,
				nullptr
			);
		}
		_swapChain = CreateSwapChain(_window);
	}

	Gpu::~Gpu() {
		_device = nullptr;
		_frameIndex = Frames;
	}

	auto Gpu::FrameIndex() const -> uint64_t {
		return _frameIndex;
	}

	auto Gpu::FenceValue(uint64_t frame) -> uint64_t {
		return _fences[_frameIndex]->GetCompletedValue();
	}

	auto Gpu::CreateSwapChain(std::optional<HWND> handle) -> winrt::com_ptr<IDXGISwapChain4> {
		if(handle.has_value()) {
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

	auto Gpu::ExecuteDirect(std::array<winrt::com_ptr<ID3D12GraphicsCommandList>, Frames> lists) -> void {
		ExecuteCommandLists(lists, D3D12_COMMAND_LIST_TYPE_DIRECT);
	}
	auto Gpu::ExecuteCopy(std::array<winrt::com_ptr<ID3D12GraphicsCommandList>, Frames> lists) -> void {
		ExecuteCommandLists(lists, D3D12_COMMAND_LIST_TYPE_COPY);
	}
	auto Gpu::ExecuteCompute(
		std::array<winrt::com_ptr<ID3D12GraphicsCommandList>,
		Frames> lists) -> void {
		ExecuteCommandLists(lists, D3D12_COMMAND_LIST_TYPE_COMPUTE);
	}
	auto Gpu::ExecuteBundle(std::array<winrt::com_ptr<ID3D12GraphicsCommandList>, Frames> lists) -> void {
		ExecuteCommandLists(lists, D3D12_COMMAND_LIST_TYPE_BUNDLE);
	}

	auto Gpu::Wait() -> void {
		_frameIndex = _swapChain->GetCurrentBackBufferIndex();

		if (_fences[_frameIndex]->GetCompletedValue() <
			_fenceValues[_frameIndex])
		{
			winrt::check_hresult(
				_fences[_frameIndex]->SetEventOnCompletion(
					_fenceValues[_frameIndex],
					_fenceEvent[0]
				)
			);

			WaitForSingleObject(_fenceEvent[0], INFINITE);
		}
		_fenceValues[_frameIndex]++;
	}

	auto Gpu::ExecuteCommandLists(
		std::array<winrt::com_ptr<ID3D12GraphicsCommandList>, Frames> lists,
		D3D12_COMMAND_LIST_TYPE type
	) -> void {

		ID3D12CommandList** raw = new ID3D12CommandList*[lists.size()];
		for (int x = 0; x < lists.size(); x++) {
			raw[x] = static_cast<ID3D12CommandList*>(lists[x].get());
		}

		ID3D12CommandQueue* temp;
		uint8_t fenceEvent;

		
		switch (type) {
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
			temp = _directQueue.get();
			fenceEvent = 0;
			break;
		case D3D12_COMMAND_LIST_TYPE_BUNDLE:
			temp = _bundleQueue.get();
			fenceEvent = 3;
			break;
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			temp = _computeQueue.get();
			fenceEvent = 2;
			break;
		case D3D12_COMMAND_LIST_TYPE_COPY:
			temp = _copyQueue.get();
			fenceEvent = 1;
			break;
		default:
			temp = _directQueue.get();
			fenceEvent = 0;
		}

		temp->ExecuteCommandLists(
			lists.size(),
			raw
		);

		winrt::check_hresult(
			temp->Signal(
				_fences[_frameIndex].get(),
				_fenceValues[_frameIndex]
			)
		);
	}
}

#include "pch.hxx"
#include "Gpu.hxx"

#include "DeviceHandler.hxx"
namespace Cyanite::GraphicsKit {
	Gpu::Gpu() {
		auto adapter = DeviceHandler::QueryAdapters();
		winrt::check_hresult(D3D12CreateDevice(
			adapter.get(),
			MIN_D3D_LVL,
			IID_PPV_ARGS(_device.put())
		));
	}

	Gpu::~Gpu() {
		_device = nullptr;
	}

	auto Gpu::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type,
		D3D12_COMMAND_QUEUE_PRIORITY priority) -> winrt::com_ptr<ID3D12CommandQueue> {
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
}

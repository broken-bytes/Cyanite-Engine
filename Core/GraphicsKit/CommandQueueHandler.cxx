#include "pch.hxx"
#include "CommandQueueHandler.hxx"


namespace Cyanite::GraphicsKit {
	auto CommandQueueHandler::BindDevice(winrt::com_ptr<ID3D12Device8> device) -> void {
		_device = device;
	}
	
	auto CommandQueueHandler::CreateCommandQueue() -> winrt::com_ptr<ID3D12CommandQueue> {
		winrt::com_ptr<ID3D12CommandQueue> queue;
		// Describe and create the command queue.
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		winrt::check_hresult(_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(queue.put())));
		return queue;
	}
	
	auto CommandQueueHandler::DisposeCommandQueue(winrt::com_ptr<ID3D12CommandQueue> queue) {
		queue->Release();
		queue = nullptr;
	}
}

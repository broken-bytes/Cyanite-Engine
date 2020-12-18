#include "pch.hxx"
#include "CommandAllocatorHandler.hxx"
#include <mutex>

using namespace winrt;

namespace Cyanite::GraphicsKit {
	CommandAllocatorHandler::CommandAllocatorHandler(D3D12_COMMAND_LIST_TYPE type) {
		_type = type;
	}

	auto CommandAllocatorHandler::BindDevice(winrt::com_ptr<ID3D12Device> device) -> void {
		_device = device;
	}

	auto CommandAllocatorHandler::Next(uint64_t fence) -> winrt::com_ptr<ID3D12CommandAllocator> {
		std::scoped_lock lock{_mutex};
		winrt::com_ptr<ID3D12CommandAllocator> alloc;
		if(!_availableAllocators.empty()) {
			auto pair = _availableAllocators.front();

			if (pair.first <= fence)
			{
				alloc = pair.second;
				winrt::check_hresult(alloc->Reset());
				_availableAllocators.pop();
			}
		}
		
		if (alloc == nullptr)
		{
			alloc = CreateAllocator();
		}

		return alloc;
	}

	auto CommandAllocatorHandler::Clear() -> void {
		for(auto& item: _allocators) {
			item->Release();
		}
		_allocators.clear();
	}

	auto CommandAllocatorHandler::Size() -> size_t {
		return _allocators.size();
	}

	auto CommandAllocatorHandler::DisposeAllocator(
		uint64_t fence,
		winrt::com_ptr<ID3D12CommandAllocator> allocator
	) -> void {
		std::scoped_lock lock{ _mutex };
		_availableAllocators.push({ fence, allocator });
	}
	
	auto CommandAllocatorHandler::CreateAllocator()->
	winrt::com_ptr<ID3D12CommandAllocator> {
		winrt::com_ptr<ID3D12CommandAllocator> alloc;
		_device->CreateCommandAllocator(
			_type,
			IID_PPV_ARGS(alloc.put()
			)
		);
		wchar_t allocatorName[32];
		swprintf(allocatorName, 32, L"CommandAllocator %zu", _allocators.size());
		alloc->SetName(allocatorName);
		return alloc;
	}
}

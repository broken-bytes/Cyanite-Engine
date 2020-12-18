#pragma once
#include "pch.hxx"

namespace Cyanite::GraphicsKit {
	class GpuResource {
	public:
		GpuResource() :
			_usageState(D3D12_RESOURCE_STATE_COMMON),
			_transitioningState(D3D12_RESOURCE_STATES(-1)),
			_gpuVirtualAddress(GPU_VIRTUAL_ADDRESS_UNDEF),
			_allocatedMemory(nullptr)
		{}

		GpuResource(
			winrt::com_ptr<ID3D12Device> resource,
			D3D12_RESOURCE_STATES state) :
			_gpuVirtualAddress(GPU_VIRTUAL_ADDRESS_UNDEF),
			_allocatedMemory(nullptr),
			_resource(resource),
			_usageState(state),
			_transitioningState(D3D12_RESOURCE_STATES(-1))
		{
		}

		[[nodiscard]] ID3D12Resource* operator->() { return Resource().get(); }

		virtual auto Dispose() -> void {
			_resource = nullptr;
			_gpuVirtualAddress = GPU_VIRTUAL_ADDRESS_UNDEF;
			if (_allocatedMemory != nullptr)
			{
				VirtualFree(_allocatedMemory, 0, MEM_RELEASE);
				_allocatedMemory = nullptr;
			}
		}

		[[nodiscard]] auto VirtualAddress() const ->D3D12_GPU_VIRTUAL_ADDRESS {
			return _gpuVirtualAddress;
		}
	protected:
		[[nodiscard]] auto Resource() const ->winrt::com_ptr<ID3D12Resource> {
			return _resource;
		}

		[[nodiscard]] auto UsageState() const ->D3D12_RESOURCE_STATES {
			return _usageState;
		}

		[[nodiscard]] auto TransitioningState() const ->D3D12_RESOURCE_STATES {
			return _transitioningState;
		}
	private:
		winrt::com_ptr<ID3D12Resource> _resource;
		D3D12_RESOURCE_STATES _usageState;
		D3D12_RESOURCE_STATES _transitioningState;
		D3D12_GPU_VIRTUAL_ADDRESS _gpuVirtualAddress;

		void* _allocatedMemory;	
	};
}

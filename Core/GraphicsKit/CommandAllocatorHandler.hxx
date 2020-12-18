#pragma once
#include <mutex>
#include <map>
#include <queue>

namespace Cyanite::GraphicsKit {
	class CommandAllocatorHandler {
	public:
		explicit CommandAllocatorHandler(D3D12_COMMAND_LIST_TYPE type);

		/// <summary>
		/// Binds to a device 
		/// </summary>
		/// <param name="device">Device which this handler will be bound to
		/// </param>
		/// <returns></returns>
		auto BindDevice(winrt::com_ptr<ID3D12Device> device) -> void;
		/// <summary>
		/// Gets a free allocator.
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		auto Next(uint64_t fence)->winrt::com_ptr<ID3D12CommandAllocator>;

		/// <summary>
		/// Clears up memory currently used by this allocator
		/// </summary>
		/// <param name="allocator">The allocator for which memory should be reusable</param>
		/// <returns></returns>
		auto DisposeAllocator(
			uint64_t fence,
			winrt::com_ptr<ID3D12CommandAllocator> allocator
		) -> void;

		/// <summary>
		/// Clears the handler, removing all allocators
		/// </summary>
		/// <returns></returns>
		auto Clear() -> void;

		/// <summary>
		/// Gets the number of allocators
		/// </summary>
		/// <returns></returns>
		auto Size()->size_t;

	private:
		std::mutex _mutex;
		winrt::com_ptr<ID3D12Device> _device;
		D3D12_COMMAND_LIST_TYPE _type;
		std::vector<winrt::com_ptr<ID3D12CommandAllocator>> _allocators;
		std::queue<
			std::pair<
			uint64_t,
			winrt::com_ptr<ID3D12CommandAllocator>
			>
		> _availableAllocators;
		/// <summary>
		/// Creates a new allocator
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		auto CreateAllocator()->winrt::com_ptr<ID3D12CommandAllocator>;
	};
}

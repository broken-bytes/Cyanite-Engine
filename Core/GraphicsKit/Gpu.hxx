#pragma once
#include "pch.hxx"

namespace Cyanite::GraphicsKit {
#ifdef _WIN32
	typedef winrt::com_ptr<ID3D12Device> GraphicsDevice;
#endif

	class Gpu {
	public:
		Gpu(HWND window);
		~Gpu();

		auto FrameIndex() const->uint64_t;
		auto FenceValue(uint64_t frame)->uint64_t;

		[[nodiscard]] auto CreateSwapChain(
			std::optional<HWND> handle
		)->winrt::com_ptr<IDXGISwapChain4>;
		
		[[nodiscard]] auto CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE type =
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			D3D12_COMMAND_QUEUE_PRIORITY priority =
			D3D12_COMMAND_QUEUE_PRIORITY_NORMAL
		)->winrt::com_ptr<ID3D12CommandAllocator>;
		
		[[nodiscard]] auto CreateCommandList(
			winrt::com_ptr<ID3D12CommandAllocator> alloc,
			D3D12_COMMAND_LIST_TYPE type =
			D3D12_COMMAND_LIST_TYPE_DIRECT
			) -> winrt::com_ptr<ID3D12GraphicsCommandList>;

		[[nodiscard]] auto CreateFence(uint64_t value = 0)->winrt::com_ptr<ID3D12Fence1>;

		auto ExecuteDirect(
			std::array<winrt::com_ptr<ID3D12GraphicsCommandList>, Frames> lists
		) -> void;
		auto ExecuteCopy(
			std::array<winrt::com_ptr<ID3D12GraphicsCommandList>, Frames> lists
		) -> void;
		auto ExecuteCompute(
			std::array<winrt::com_ptr<ID3D12GraphicsCommandList>, Frames> lists
		) -> void;
		auto ExecuteBundle(
			std::array<winrt::com_ptr<ID3D12GraphicsCommandList>, Frames> lists
		) -> void;

		auto Wait() -> void;
		
	private:
		GraphicsDevice _device;
		HWND _window;
		winrt::com_ptr<IDXGISwapChain4> _swapChain;

		std::array<winrt::com_ptr<ID3D12Fence1>, Frames> _fences;
		std::array<uint64_t, Frames> _fenceValues;
		uint64_t _frameIndex;

		std::array<HANDLE, CommandQueues> _fenceEvent;
		
		winrt::com_ptr<ID3D12CommandQueue> _directQueue;
		winrt::com_ptr<ID3D12CommandQueue> _copyQueue;
		winrt::com_ptr<ID3D12CommandQueue> _computeQueue;
		winrt::com_ptr<ID3D12CommandQueue> _bundleQueue;

		auto ExecuteCommandLists(
			std::array<winrt::com_ptr<ID3D12GraphicsCommandList>, Frames> lists,
			D3D12_COMMAND_LIST_TYPE type
		) -> void;
	
		auto CreateCommandQueue(
			D3D12_COMMAND_LIST_TYPE type =
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			D3D12_COMMAND_QUEUE_PRIORITY priority =
			D3D12_COMMAND_QUEUE_PRIORITY_NORMAL
		)->winrt::com_ptr<ID3D12CommandQueue>;
	};
}
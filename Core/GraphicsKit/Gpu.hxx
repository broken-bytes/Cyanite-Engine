#pragma once
#include "pch.hxx"

namespace Cyanite::GraphicsKit {
#ifdef _WIN32
	typedef winrt::com_ptr<ID3D12Device> GraphicsDevice;
#endif

	class Gpu {
	public:
		Gpu();
		~Gpu();

		auto CreateCommandQueue(
			D3D12_COMMAND_LIST_TYPE type =
				D3D12_COMMAND_LIST_TYPE_DIRECT,
			D3D12_COMMAND_QUEUE_PRIORITY priority =
				D3D12_COMMAND_QUEUE_PRIORITY_NORMAL
		)->winrt::com_ptr<ID3D12CommandQueue>;
		auto CreateCommandList(
			D3D12_COMMAND_LIST_TYPE type =
			D3D12_COMMAND_LIST_TYPE_DIRECT) -> winrt::com_ptr<ID3D12GraphicsCommandList>;

	private:
		GraphicsDevice _device;
	};
}
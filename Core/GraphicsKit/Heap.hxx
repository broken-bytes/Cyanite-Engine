#pragma once

namespace Cyanite::GraphicsKit {
	class Heap
	{
	public:
		[[nodiscard]] auto Start() const->uint64_t;
		[[nodiscard]] auto Size() const->size_t;
		[[nodiscard]] auto Offset() const->uint64_t;
		auto Allocate() -> void;

	private:
		uint64_t _start;
		size_t _size;
		uint64_t _offset;
		winrt::com_ptr<ID3D12DescriptorHeap> _heap;
	};
}


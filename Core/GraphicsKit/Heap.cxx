#include "pch.hxx"
#include "Heap.hxx"

namespace Cyanite::GraphicsKit {

	auto Heap::Start() const -> uint64_t {
		return _start;
	}
	auto Heap::Size() const -> size_t {
		return _size;
	}
	auto Heap::Offset() const -> uint64_t {
		return _offset;
	}
	auto Heap::Allocate() -> void {}
}
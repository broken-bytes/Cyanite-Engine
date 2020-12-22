#pragma once
#include <cstdint>
#include <functional>
#include <optional>

enum class EventType {
	Generic,
	GameInput,
	WindowResize,
	WindowFullScreen
};


struct IEvent {
	EventType Type;
	void* Sender;
	void* Data;
	std::function<void(void*)> Callback;
	
	bool Handled;
};

template<
	typename S,
	typename D,
	typename C
>
struct Event : IEvent {
	S Sender;
	D Data;
	std::function<void(C)> Callback;
};

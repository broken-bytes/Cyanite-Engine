#pragma once
#include "pch.hxx"
#include "Event.hxx"
#include "EventQueue.hxx"

class EventRelay {
public:
	EventRelay(std::function<void(IEvent)> callback) {
		_callback = callback;

		_pullThread = std::thread([this] { this->PullEvents(); });
	}
private:
	std::thread _pullThread;
	std::function<void(IEvent)> _callback;

	auto PullEvents() -> void {
		for(auto& item: EventQueue::Receive()) {
			_callback(item);
		}
	}
};

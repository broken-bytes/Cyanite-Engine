#pragma once
#include "Event.hxx"

class EventQueue {
public:
	static inline auto Push(IEvent event) {
		std::scoped_lock lock{ _mutex };
		_queue.emplace_back(event);
	}

	static inline auto Receive() ->std::vector<IEvent> {
		return _queue;
	}

private:
	static std::vector<IEvent> _queue;
	static std::mutex _mutex;
};

#pragma once

#include <chrono>
#include <winsock2.h>

struct Connection {
public:
	SOCKADDR_IN Address;
	std::chrono::system_clock::time_point TimeLastUpdate;
	inline bool operator==(const Connection& rhs) {
		return this->Address.sin_addr.S_un.S_addr ==
			rhs.Address.sin_addr.S_un.S_addr && this->Address.sin_port ==
			rhs.Address.sin_port;
	}
};

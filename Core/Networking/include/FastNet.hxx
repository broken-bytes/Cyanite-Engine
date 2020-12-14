#pragma once

#include <cstdint>
#include <string>
#include <functional>


namespace BrokenBytes::FastNet {
	enum class Error : int {
		OK = 000,
		CLIENT_NOT_CREATED = 100,
		CLIENT_ALREADY_CREATED = 101,
		CLIENT_ALREADY_CONNECTED = 102,
		CLIENT_CONNECTION_TIMEOUT = 103,
		SERVER_NOT_CREATED = 200,
		SERVER_ALREADY_CREATED = 201,
		INTERNAL_ERROR = 300
	};

	namespace Peers {
		class Client;
		class Server;
	}

	void Connect(std::string address, uint16_t port);
	[[nodiscard]] Peers::Client* Client();
	[[nodiscard]] Error LastError();
	[[nodiscard]] Peers::Server* Server();
	[[nodiscard]] Peers::Client* StartClient();
	[[nodiscard]] Peers::Server* StartServer(uint16_t port, uint16_t maxClients);
	void StopClient();
	void StopServer();
}
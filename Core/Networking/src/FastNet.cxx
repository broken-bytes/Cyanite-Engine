#include <thread>
#include <chrono>
#include <memory>
#include <iostream>

#include "../include/FastNet.hxx"
#include "../include/Types.hxx"
#include "../include/Socket.hxx"
#include "../include/Macros.hxx"
#include "../include/Peer.hxx"
#include "../include/Client.hxx"
#include "../include/Server.hxx"
#include "../include/Logger.hxx"

namespace Types = BrokenBytes::FastNet::Types;

namespace BrokenBytes::FastNet {
	std::unique_ptr<Peers::Client> _client;
	std::unique_ptr<Peers::Server> _server;
	Error _lastError = Error::OK;
	void Connect(std::string address, uint16_t port) {
		if (_client->Connected()) {
			Logger::Write("Client already connected", Logger::Level::Error);
			_lastError = Error::CLIENT_ALREADY_CONNECTED;
		}
		else {
			_lastError = _client->Connect(address, port);
		}
	}

	Peers::Client* Client() {
		return _client.get();
	}

	Error LastError() {
		return _lastError;
	}


	Peers::Server* Server() {
		return _server.get();
	}

	Peers::Client* StartClient() {
		if (_client == nullptr) {
			_client = std::make_unique<Peers::Client>();
			_lastError = Error::OK;
			return _client.get();
		} else {
			_lastError = Error::CLIENT_ALREADY_CREATED;
			throw std::exception("Could not start client");
		}
	}

	Peers::Server* StartServer(uint16_t port, uint16_t maxClients) {
		if (_server == nullptr) {
			_server = std::make_unique<Peers::Server>(
				port,
				maxClients
			);
			_lastError = Error::OK;
			return _server.get();
		}
		else {
			_lastError = Error::SERVER_ALREADY_CREATED;
			throw std::exception("Could not start server");
		}
		
	}

	void StopClient() {
		if (_client.get() != nullptr) {
			auto client = _client.release();
			delete client;
		}
	}

	void StopServer() {
		if (_server.get() != nullptr) {
			auto server = _server.release();
			delete server;
		}
	}
}



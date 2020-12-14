#pragma once

#include <iostream>

#include "../include/Server.hxx"
#include "../include/Helpers.hxx"
#include "../include/Peer.hxx"

namespace BrokenBytes::FastNet::Peers {
	Server::Server(uint16_t port, uint16_t maxClients) : Peer(port) {
		_maxClients = maxClients;
		_clients = std::vector<Connection>();
		_clients.reserve(_maxClients);
		_port = port;
		_worker = std::thread(&Server::Read, this);
		_connectionEvent = nullptr;
	}

	Server::~Server() {
		_worker.join();
	}

	Server::Server(const Server& other) {
		this->_clients = other._clients;
		this->_maxClients = other._maxClients;
		this->_port = other._port;
		this->_worker = std::thread(&Server::Read, this);
		this->_connectionEvent = other._connectionEvent;
	}

	Server::Server(Server&& other) {
		this->_clients = other._clients;
		this->_maxClients = other._maxClients;
		this->_port = other._port;
		this->_worker = std::thread(&Server::Read, this);
		this->_connectionEvent = other._connectionEvent;
	}

	// Send to all clients
	void Server::Broadcast(const std::string& event, const std::string& data, Types::Channel channel) {
		for (int x = 0; x < _clients.size(); x++) {
			Send(x, event, data, channel);
		}
	}


	void Server::SetConnectionEvent(std::function<void(bool, uint16_t)> event) {
		_connectionEvent = event;
	}

	void Server::Send(uint16_t clientId, std::string event, std::string data, Types::Channel channel) {
		auto client = this->_clients[clientId];
		uint16_t port;
		std::string addr;
		Helpers::RawAddressToAdressPort(client.Address, addr, port);
		Peer::Send(event + data, addr, port, channel);
	}

	void Server::Read() {
		while (Active()) {
			auto packages = Received();
			for (auto package : packages) {
				std::string data;
				std::string event;

				// CONNECT REQUEST
				if (package.Payload == std::string(CONNECT_REQ)) {
					if (_clients.size() >= _maxClients) {
						uint16_t port;
						std::string addr;
						Helpers::RawAddressToAdressPort(package.Address, addr, port);
						Peer::Send(
							std::string(CONNECT_REJ),
							addr,
							port,
							Types::Channel::Unreliable
						);
						continue;
					}
					bool isExistingClient = false;
					for (int x = 0; x < _clients.size(); x++) {
						// Entry exists already
						if (Helpers::CompareSockAddrIn(
							_clients[x].Address,
							package.Address)
							) {
							isExistingClient = true;
						}
					}

					if(isExistingClient) {
						continue;
					}
					
					std::string addr;
					uint16_t p;
					Helpers::RawAddressToAdressPort(
						package.Address,
						addr,
						p
					);

					_clients.push_back(
						Connection{
							package.Address,
							std::chrono::system_clock::now()
						}
					);
					_connectionEvent(true, _clients.size());
					Peer::Send(std::string(CONNECT_ACK), addr, p, Types::Channel::Unreliable);
				} // CONNECT REQUEST END
				else {
					Helpers::GetEventData(package.Payload, event, data);

					if (auto func = this->Receiver()) {
						func(event, data);
					}
					else {
					}
				}
				for (auto item : _clients) {
					if (Helpers::CompareSockAddrIn(
						item.Address,
						package.Address
					)) {
						item.TimeLastUpdate = std::chrono::system_clock::now();
					}
				}
			}

			for (auto client : _clients) {
				auto now = std::chrono::system_clock::now();
				std::chrono::duration<double> diff = client.TimeLastUpdate - now;
				if (diff.count() * 1000 > CONNECT_TIMEOUT_ms) {
					_clients.erase(
						std::remove(
							_clients.begin(), _clients.end(), client
						),
						_clients.end()
					);
				}
			}
			std::this_thread::sleep_for(
				std::chrono::duration<double, std::milli>(5)
			);
		}
	}

}

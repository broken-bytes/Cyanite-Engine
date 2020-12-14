#pragma once

#include <string>
#include <functional>


#include "Connection.hxx"
#include "Types.hxx"
#include "Socket.hxx"
#include "Peer.hxx"

namespace BrokenBytes::FastNet::Peers {
	class Server : public Peer {
	public:
		Server(uint16_t port, uint16_t maxClients);
		~Server();
		Server(const Server& other);
		Server(Server&& other);
		void Broadcast(const std::string& event, const std::string& data, Types::Channel channel);
		void SetConnectionEvent(std::function<void(bool, uint16_t)> event);
		void Send(uint16_t clientId, std::string event, std::string data, Types::Channel channel);
	private:
		uint16_t _port = 0;
		uint16_t _maxClients;
		std::vector<Connection> _clients;
		std::function<void(bool, uint16_t)> _connectionEvent;
		std::thread _worker;
		void Read() override;
	};
}
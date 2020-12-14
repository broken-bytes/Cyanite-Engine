#pragma once

#include <string>
#include <functional>


#include "Connection.hxx"
#include "Socket.hxx"
#include "Types.hxx"
#include "Peer.hxx"

namespace BrokenBytes::FastNet::Peers {
	class Client : public Peer {
	public:
		Client();
		~Client();
		Client(const Client& other);
		Error Connect(std::string address, uint16_t port);
		void Disconnect();
		bool Connected() const;
		void Send(std::string event, std::string data, Types::Channel channel);
	private:
		std::thread _worker;
		Connection _server;
		void Read() override;
	};
}
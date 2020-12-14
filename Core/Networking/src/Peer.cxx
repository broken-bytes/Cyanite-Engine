#include <iostream>

#include "../include/Peer.hxx"

namespace BrokenBytes::FastNet::Peers {
	Peer::Peer() : Peer(0) {
	}

	Peer::Peer(uint16_t port) {
		_socket = std::make_unique<Internal::Socket>(port);
	}

	Peer::~Peer() {
		std::cout << "PEER DESTR" << std::endl;
	}

	Peer::Peer(Peer&& other) {
		this->_receiver = other._receiver;
		this->_port = other._port;
		this->_socket = std::move(other._socket);
	}
	
	bool Peer::Active() const {
		return _socket->Active();
	}

	Error Peer::Connect(std::string address, uint16_t port) {
		return _socket->Connect(address, port);
	}

	bool Peer::Connected() const {
		return _socket->Connected();
	}

	void Peer::Disconnect() {
		_socket->Disconnect();
	}

	EventFunc Peer::Receiver() const {
		return _receiver;
	}

	std::vector<BrokenBytes::FastNet::Internal::PackageRaw> Peer::Received() {
		return _socket->Received();
	}

	void Peer::Emit(
		std::string event,
		std::string data,
		Types::Channel channel
	) const {
		_socket->Send(
			event + MSG_SECTION_DELIMITER + data, channel
		);
	}
	
	void Peer::SetReceiver(EventFunc receiver) {
		_receiver = receiver;
	}

	void Peer::Send(
		std::string data,
		std::string address,
		uint16_t port,
		Types::Channel channel
	) const {
		_socket->Send(data, address, port, channel);
	}
}
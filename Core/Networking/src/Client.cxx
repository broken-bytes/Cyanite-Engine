#include "../include/Client.hxx"
#include "../include/Logger.hxx"
#include "../include/Helpers.hxx"

namespace BrokenBytes::FastNet::Peers {

	Client::Client() : Peer(0) {
		_worker = std::thread(&Client::Read, this);
	}

	Client::~Client() {
	}

	Client::Client(const Client& other) {
	}

	Error Client::Connect(std::string address, uint16_t port) {
		auto err = Peer::Connect(address, port);

		if(err == Error::OK) {
			SOCKADDR_IN addr;
			Helpers::AdressPortToRawAddress(addr, address, port);
			_server = Connection{ addr };
		}
		return err;
	}

	void Client::Disconnect() {
		Peer::Disconnect();
	}

	void Client::Read() {
		while (Active()) {
			auto packages = this->Received();
			for (auto package : packages) {
				std::string event;
				std::string data;
				Helpers::GetEventData(package.Payload, event, data);
				Receiver()(event, data);
				Logger::Write(package.Payload);
			}
			std::this_thread::sleep_for(
				std::chrono::duration<double, std::milli>(5)
			);
		}
	}

	bool Client::Connected() const {
		return Peer::Connected();
	}

	void Client::Send(std::string event, std::string data, Types::Channel channel) {
		std::string addr;
		uint16_t port;
		Helpers::RawAddressToAdressPort(_server.Address, addr, port);
		Peer::Send(event + data, addr, port, channel);
	}
}

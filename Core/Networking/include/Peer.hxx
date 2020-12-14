#pragma once 

#include <functional>

#include "Types.hxx"
#include "Socket.hxx"


typedef std::function<void(std::string, std::string)> EventFunc;

namespace BrokenBytes::FastNet::Peers {
	class Peer {
	public:
		Peer();
		Peer(uint16_t port);
		virtual ~Peer();
		Peer(Peer&& other);
		void Emit(
			std::string event,
			std::string data,
			Types::Channel channel
		) const;
		void SetReceiver(EventFunc receiver);

	protected:
		bool Active() const;
		virtual void Read() = 0;
		Error Connect(std::string address, uint16_t port);
		bool Connected() const;
		void Disconnect();
		EventFunc Receiver() const;
		std::vector<BrokenBytes::FastNet::Internal::PackageRaw> Received();
		void Send(
			std::string data,
			std::string address,
			uint16_t port,
			Types::Channel channel
		) const;
		
	private:
		uint16_t _port = 0;
		EventFunc _receiver;
		std::unique_ptr<Internal::Socket> _socket;
	};
}
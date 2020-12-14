#pragma once

#include <cstdint>
#include <map>
#include <thread>
#include <vector>
#include <WinSock2.h>
#include <mutex>
#include <memory>
#include <string>

#include "Defines.hxx"
#include "FastNet.hxx"
#include "Types.hxx"

namespace BrokenBytes::FastNet::Internal {
	struct PackageRaw {
		SOCKADDR_IN Address;
		/// CHANNEL MSG_ID LAST EVENT DATA
		std::string Payload;
	};

	struct Package {
		SOCKADDR_IN Address;
		uint64_t ID;
		// Used for Sequenced Channel to determine which package needs to arrive for this package to be valid
		uint64_t LastID;
		Types::Channel Channel;
		std::string Event;
		std::string Data;
	};
	

	struct Host {
		std::string Address;
		uint16_t Port = 0;
		std::vector<uint64_t> _receivedReliable;
		std::vector< uint64_t> _processedSequenceIds;
		std::vector<PackageRaw> _receivedRSequenced;
		uint64_t _lastSequenced = 0;
	};

	struct Socket {
	public:
		Socket();
		~Socket();
		Socket(uint16_t port);
		Socket(Socket&& other);
		Socket& operator=(Socket& other);
		bool Active() const;
		Error Connect(const std::string& address, const uint16_t port);
		bool Connected() const;
		void Disconnect();
		std::vector<PackageRaw> Received();
		void Send(const std::string& data, Types::Channel channel);
		void Send(
			const std::string& data,
			const std::string& address,
			const uint16_t port,
			Types::Channel channel
		);

	private:
		int _conn_msElapsed;
		uint64_t _packageCount = 0;
		bool _isConnected = false;
		std::unique_ptr<SOCKET> _socket;
		std::vector<Host> _hosts;
		std::thread _receiveThread;
		std::thread _sendThread;
		bool _isActive = false;
		std::vector<PackageRaw> _received;
		mutable std::mutex _readMutex;
		std::vector<PackageRaw> _queue;
		std::map<uint64_t, PackageRaw> _reliableCache;
		mutable std::mutex _sendMutex;
		uint64_t _lastSequenced;
		void Create(uint16_t port);
		void Receive();
		void Transmit();
	};
}
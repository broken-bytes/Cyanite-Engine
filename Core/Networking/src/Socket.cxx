#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif 


#include "../include/Socket.hxx"
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <cassert>
#include <iostream>
#include <future>
#include <mutex>
#include <algorithm>


#include "../include/Macros.hxx"
#include "../include/Defines.hxx"
#include "../include/Helpers.hxx"
#include "../include/Logger.hxx"

#pragma comment(lib, "Ws2_32.lib")

namespace BrokenBytes::FastNet::Internal {
	static bool WINSOCK_INIT = false;
	Socket::Socket() : Socket(0) {
	}

	Socket::Socket(uint16_t port) {
		_packageCount = 0;
		_isActive = false;
		_received = std::vector<PackageRaw>();
		_received.reserve(256);
		_queue = std::vector<PackageRaw>();
		_queue.reserve(256);
		_reliableCache = std::map<uint64_t, PackageRaw>();
		Create(port);
	}

	Socket::~Socket() {
		this->_isActive = false;
	}

	Socket::Socket(Socket&& other) {
		this->_isActive = other._isActive;
		this->_isConnected = other._isConnected;
		this->_packageCount = other._packageCount;
		this->_socket = std::move(other._socket);
	}

	Socket& Socket::operator=(Socket& other) {
		return other;
	}

	bool Socket::Active() const {
		return _isActive;
	}

	void Socket::Create(uint16_t port = 0) {
		if (!WINSOCK_INIT) {
			WSADATA wsaData;
			auto code = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (code != 0) {
				Logger::Write("WinSock failed to init", Logger::Level::Error);
			}
			else {
				Logger::Write("WinSock initialized", Logger::Level::Info);
				WINSOCK_INIT = true;
			}
		}

		_socket = std::make_unique<SOCKET>(socket(
			AF_INET,
			SOCK_DGRAM,
			0
		));
		// Create a SOCKET for connecting to server
		if (port != 0) {
			SOCKADDR_IN addr;
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			addr.sin_addr.s_addr = ADDR_ANY;
			bind(
				*_socket,
				reinterpret_cast<SOCKADDR*>(&addr),
				sizeof(addr)
			);
			_isActive = true;
			_receiveThread = std::thread(&Socket::Receive, this);
			_sendThread = std::thread(&Socket::Transmit, this);
		}

		if (*_socket == INVALID_SOCKET) {
			WSACleanup();
			auto err = WSAGetLastError();
			Logger::Write("Invalid Socket: " + err, Logger::Level::Error);
		}
		else {
			Logger::Write("SOCKET CREATED");
		}
	}

	Error Socket::Connect(const std::string& address, const uint16_t port) {
		std::scoped_lock lock{ _readMutex, _sendMutex };
		auto killSwitch = std::async([this]() {

			std::this_thread::sleep_for(std::chrono::milliseconds(250));
			if (_conn_msElapsed >= CONNECT_TIMEOUT_ms) {
				auto sock = _socket.release();
				delete sock;
				_socket = std::make_unique<SOCKET>(socket(
					AF_INET,
					SOCK_DGRAM,
					0
				));
				// Create a SOCKET for connecting to server
			}
			else {
				_conn_msElapsed += 250;
			}
			});
		std::string conReq(CONNECT_REQ);
		SOCKADDR_IN target;
		Helpers::AdressPortToRawAddress(target, address, port);
		sendto(
			*_socket,
			conReq.c_str(),
			conReq.length(),
			0,
			reinterpret_cast<SOCKADDR*>(&target),
			sizeof(SOCKADDR_IN)
		);

		char buffer[MAX_PAYLOAD_LENGTH];
		SOCKADDR_IN sender;
		int senderSize = sizeof(SOCKADDR_IN);
		std::string str;
		do {
			auto bytes = recvfrom(
				*_socket,
				buffer,
				sizeof(buffer),
				0,
				reinterpret_cast<SOCKADDR*>(&sender),
				&senderSize
			);

			auto err = WSAGetLastError();
			str = std::string(buffer, bytes);

			if (bytes == SOCKET_ERROR) {
				std::cerr << "SOCKET_ERROR" << std::endl;
				return Error::INTERNAL_ERROR;
			}
		} while (str != std::string(CONNECT_ACK) && _conn_msElapsed < CONNECT_TIMEOUT_ms);
		if(_conn_msElapsed > CONNECT_TIMEOUT_ms)
		{
			_conn_msElapsed = 0;
			return Error::CLIENT_CONNECTION_TIMEOUT;
		}
		_isConnected = true;
		Logger::Write("Connection established");
		_hosts.push_back(Host{ address, port });
		_isActive = true;
		_receiveThread = std::thread(&Socket::Receive, this);
		_sendThread = std::thread(&Socket::Transmit, this);
		return Error::OK;
	}

	bool Socket::Connected() const {
		return _isConnected;
	}

	void Socket::Disconnect() {
		closesocket(*_socket);
		_isActive = false;
	}

	std::vector<PackageRaw> Socket::Received() {
		std::scoped_lock lock{ _readMutex };
		auto copy = _received;
		_received.clear();
		return copy;
	}


	void Socket::Receive() {
		while (_isActive) {
			SOCKADDR_IN sender;
			int senderSize = sizeof(SOCKADDR_IN);
			char buffer[MAX_PAYLOAD_LENGTH];

			auto bytes = recvfrom(
				*_socket,
				buffer,
				MAX_PAYLOAD_LENGTH,
				0,
				reinterpret_cast<SOCKADDR*>(&sender),
				&senderSize
			);

			std::string str(buffer, bytes);
			Logger::Write("Received: " + str);
			std::scoped_lock lock{ _readMutex };

			auto pack = PackageRaw{ sender, str };

			// Check if MSG_ACK package
			if (pack.Payload.substr(0, sizeof(MSG_ACK) - 1) == MSG_ACK) {
				auto strId = pack.Payload.substr(sizeof(MSG_ACK) - 1, pack.Payload.length() - sizeof(MSG_ACK) + 1);
				uint64_t id = std::stoi(strId);
				std::cout << id << std::endl;
				_reliableCache.erase(id);
				continue;
			}

			// Check if connect request package
			// Let Server handle req messages
			if (pack.Payload == CONNECT_REQ) {
				_received.push_back(pack);
				continue;
			}
			
			auto temp = Helpers::GetPackageFromRaw(pack);

			auto channel = "";
			switch (temp.Channel)
			{
			case Types::Channel::Unreliable:
				channel = "Unreliable";
				break;
			case Types::Channel::Reliable:
				channel = "Reliable";
				break;
			case Types::Channel::ReliableSequenced:
				channel = "Reliable Sequenced";
				break;
			}
			
			// Acknowledge reliable messages at the beginning
			if(temp.Channel == Types::Channel::Reliable
				|| temp.Channel == Types::Channel::ReliableSequenced) {
				std::string addr;
				uint16_t p;
				Helpers::RawAddressToAdressPort(pack.Address, addr, p);
				Send(MSG_ACK + temp.ID, addr, p, Types::Channel::Unreliable);
			}

			if(temp.Channel == Types::Channel::ReliableSequenced) {
				int id = 0;
				for(int x = 0; x < _hosts.size(); x++) {
					std::string addr;
					uint16_t port;
					Helpers::RawAddressToAdressPort(temp.Address, addr, port);
					if(_hosts[x].Port == port && _hosts[x].Address == addr) {
						id = x;
						break;
					}
				}

			// THE LAST PACK ID IS NOT 0, THUS IT IS A VALID PACK ID
				if(temp.ID != 0) {
					// THE LAST PACK ID WAS ALREADY PROCESSED, THIS PACK ARRIVED IN ORDER
					if(std::find(_hosts[id]._processedSequenceIds.begin(),
						_hosts[id]._processedSequenceIds.end(),
						temp.LastID) != _hosts[id]._processedSequenceIds.end()) {
						_hosts[id]._processedSequenceIds.push_back(temp.ID);
						_received.push_back(pack);
					}
					// THE PACK WAS NOT FOUND IN PROCESSED IDs
					else {
						_hosts[id]._receivedRSequenced.push_back(pack);
						std::sort(
							_hosts[id]._receivedRSequenced.begin(),
							_hosts[id]._receivedRSequenced.end(),
							[](PackageRaw left, PackageRaw right) {
								auto l = Helpers::GetPackageFromRaw(left);
								auto r = Helpers::GetPackageFromRaw(right);
								return l.ID < r.ID;
							}
						);
						// LOOP OVER RECEIVED PACKS AND SEE IF MATCHING PACKS ARE PRESENT
						for(int x = 1; x < _hosts[id]._receivedRSequenced.size(); x++) {
							auto last = Helpers::GetPackageFromRaw(
								_hosts[id]._receivedRSequenced[x - 1]
							);
							auto current = Helpers::GetPackageFromRaw(
								_hosts[id]._receivedRSequenced[x]
							);
							if(last.ID == current.LastID) {
								_received.push_back(_hosts[id]._receivedRSequenced[x]);
							}
						}
					}
				}
				// THE PACK ID IS 0, THIS IS THE FIRST PACK
				else {
					_hosts[id]._processedSequenceIds.push_back(temp.ID);
					_received.push_back(pack);
				}
			}
			_received.push_back(pack);
			std::this_thread::sleep_for(
				std::chrono::duration<double, std::milli>(1)
			);
		}
	}

	// Send to server. Does not take address as clients should not send data to each other
	void Socket::Send(const std::string& data, Types::Channel channel) {
		Send(data, _hosts[0].Address, _hosts[0].Port, channel);
	}

	// Send to specific address, used for server
	void Socket::Send(
		const std::string& data,
		const std::string& address,
		const uint16_t port,
		Types::Channel channel
	) {
		SOCKADDR_IN target;
		target.sin_family = AF_INET;
		target.sin_port = htons(port);
		target.sin_addr.S_un.S_addr = inet_addr(address.c_str());
		const PackageRaw pack{
			target,
			Helpers::StringFromChannel(channel)
			+ MSG_SECTION_DELIMITER
			+ std::to_string(_packageCount)
			+ ((channel == Types::Channel::ReliableSequenced)
			? std::to_string(_lastSequenced)
			: "")
			+ MSG_SECTION_DELIMITER
			+ data
		};
		_lastSequenced = _packageCount;
		_packageCount++;
		std::scoped_lock lock{ _sendMutex };
		_queue.push_back(pack);

		if(channel == Types::Channel::Reliable 
			|| channel == Types::Channel::ReliableSequenced) {
			_reliableCache.insert({ _packageCount, pack });
		}
	}

	void Socket::Transmit() {
		while (_isActive) {
			std::this_thread::sleep_for(
				std::chrono::duration<double, std::micro>(SERVER_SLEEP_us)
			);
			std::scoped_lock lock{ _sendMutex };
			// Re-add all reliable packages that didn't get acknowledged on every transmit iteration
			for (auto item : _reliableCache) {
				_queue.push_back(item.second);
			}
			for (auto item : _queue) {
				sendto(
					*_socket,
					item.Payload.c_str(),
					item.Payload.length(),
					0,
					reinterpret_cast<SOCKADDR*>(&item.Address),
					sizeof(SOCKADDR_IN)
				);
			}
			_queue.clear();
		}
	}
}
#pragma once

#include <WinSock2.h>
#include <string>

#include "Defines.hxx"

namespace BrokenBytes::FastNet::Helpers {
	inline void RawAddressToAdressPort(
		SOCKADDR_IN raw,
		std::string& address,
		uint16_t& port) {
		address = inet_ntoa(raw.sin_addr);
		port = htons(raw.sin_port);
	}

	inline void AdressPortToRawAddress(
		SOCKADDR_IN& raw,
		std::string address,
		uint16_t port) {
		raw.sin_family = AF_INET;
		raw.sin_port = htons(port);
		raw.sin_addr.S_un.S_addr = inet_addr(address.c_str());
	}

	inline bool CompareSockAddrIn(SOCKADDR_IN lhs, SOCKADDR_IN rhs) {
		return
			(lhs.sin_addr.S_un.S_addr == rhs.sin_addr.S_un.S_addr) &&
			(lhs.sin_port == rhs.sin_port);
	}

	inline void GetEventData(
		std::string message,
		std::string& event,
		std::string& data
	) {
		auto delimiter = message.find_first_of(':');
		event = message.substr(0, delimiter);
		data = message.substr(
			delimiter + 1,
			message.size() - delimiter
		);
	}

	inline std::string StringFromChannel(Types::Channel channel) {
		std::string c = "";
		switch (channel)
		{
		case Types::Channel::Unreliable:
			c = '0';
			break;
		case Types::Channel::Reliable:
			c = '1';
			break;
		case Types::Channel::ReliableSequenced:
			c = '2';
			break;
		}
		return c;
	}

	inline Types::Channel ChannelFromString(std::string channel) {
		if (channel == "0")
		{
			return Types::Channel::Unreliable;
		}

		if (channel == "1")
		{
			return Types::Channel::Reliable;
		}

		if (channel == "2")
		{
			return Types::Channel::ReliableSequenced;
		}

		return Types::Channel::Unreliable;
	}

	
	inline Internal::Package GetPackageFromRaw(Internal::PackageRaw raw) {
		auto delimiter = std::string(MSG_SECTION_DELIMITER);
		std::string s = raw.Payload;
		size_t pos = 0;
		Internal::Package pack;
		pack.Address = raw.Address;
		auto channel = s.substr(0,1);
		auto id = s.substr(1, 10);
		auto lastRId = s.substr(11, 10);
		auto data = s.substr(21, 950);
		pack.ID = std::stoi(id);
		pack.LastID = std::stoi(lastRId);
		if(channel == "0") {
			pack.Channel = Types::Channel::Unreliable;
		}
		if(channel == "1") {
			pack.Channel = Types::Channel::Reliable;
		}
		if(channel == "2") {
			pack.Channel = Types::Channel::ReliableSequenced;
		}
		
		while ((pos = data.find(delimiter)) != std::string::npos) {
			auto event = s.substr(0, pos);
			auto content = s.substr(pos, data.length() - pos);
			pack.Event = event;
			pack.Data = content;
		}
	
		return pack;
	}
}
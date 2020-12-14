#include <WinSock2.h>
#include <cstdint>
#include <thread>
#include <string>

#include "Defines.hxx"
#include "Types.hxx"


namespace BrokenBytes::FastNet::NetworkLayer {
	enum class PackageType {
		Ack,       // ac
		Ping,      // pi
		Raw,       // ra
		Vector2,   // v2
		Vector3,   // v3
		Vector4,   // v4
		Int,       // in
		Float,     // fl
		Double,    // do
		Text,      // tx
		CMD,       // cm
		RPC        // rp
	};

	// A network package is max 1024 bytes
	// Channel PackageID EventData(JSON)
	//    1      10     960   

	struct Package {
		uint16_t ClientId;
		PackageType Type;
		int Channel;
		char Data[MAX_PAYLOAD_LENGTH];
	};



	bool Connect(std::string address, uint16_t port);
	void Disconnect();
	void ReadClient();
	void ReadServer();
	void SetClientEmitter(std::function<std::string> func);
	void SetServerEmitter(std::function<std::string> func);
	void Send(std::string data, Types::Channel channel);
	void Send(std::string data, Types::Channel channel, uint16_t clientId);
	void StartClient();
	void StartServer(uint16_t port, uint16_t maxClients);
	void StopClient();
	void StopServer();
}
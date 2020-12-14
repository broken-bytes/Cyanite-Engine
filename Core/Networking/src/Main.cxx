#include "../include/FastNet.hxx"
#include "../include/Types.hxx"
#include "../include/Client.hxx"
#include "../include/Server.hxx"
#include <chrono>
#include <thread>
#include <cassert>
#include <iostream>

namespace FastNet = BrokenBytes::FastNet;





int main() {
	// MAKE UNIQUE SERVER PEER CRASHES
	auto server = FastNet::StartServer(8090, 32);
	server->SetConnectionEvent([server](bool connected, uint32_t id) {
		server->Broadcast("event", "test", FastNet::Types::Channel::Reliable);
	});
	std::cout << static_cast<int>(FastNet::LastError()) << std::endl;
	while (true) {
		
	}
}
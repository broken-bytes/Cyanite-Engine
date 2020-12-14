#include <memory>
#include <algorithm>
#include <functional>


#include "../include/NetworkLayer.hxx"
#include "../include/Macros.hxx"
#include "../include/Socket.hxx"
#include "../include/Helpers.hxx"
#include "../include/Logger.hxx"

namespace Internal = BrokenBytes::FastNet::Internal;

namespace BrokenBytes::FastNet::NetworkLayer {
	Internal::Socket* _socketC;
	Internal::Socket* _socketS;
	std::thread* _clientWorker;
	std::thread* _serverWorker;
	bool _serverActive = false;
	bool _clientActive = false;
	std::function<std::string> _emitterC;
	std::function<std::string> _emitterS;
	uint16_t _maxClients = 0;
	std::vector<Client> _clients;

	bool Connect(std::string address, uint16_t port) {
		return _socketC->Connect(address, port);
	}

	void Disconnect() {
		_socketC->Disconnect();
	}

	void ReadServer() {
		
	}

	void SetClientEmitter(std::function<std::string> func) {
		_emitterC = func;
	}

	void SetServerEmitter(std::function<std::string> func) {
		_emitterS = func;
	}

	void StartServer(uint16_t port, uint16_t maxClients) {
		
	}
}
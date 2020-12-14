#include "pch.hxx"
#include "Game.hxx"
#include "../GraphicsKit/GraphicsHandler.hxx"

namespace Cyanide::GameKit {
	Game::Game(HWND handle) {
		_handle = handle;
		_graphics = std::make_unique<GraphicsKit::GraphicsHandler>(handle);
	}
	
	auto Game::StartUp() -> void {
		_graphics->Initialize();
	}

	auto Game::CleanUp() -> void {
		_graphics->Deinitialize();
	}

	auto Game::Shutdown() -> void {
		_graphics->Deinitialize();
	}

	auto Game::OnInit() -> void {
	}
	
	auto Game::OnUpdate() -> void {
		_graphics->Update();
	}
	
	auto Game::OnRender() -> void {
		_graphics->Render();
	}
	
	auto Game::OnDestroy() -> void {
		this->Shutdown();
	}
}

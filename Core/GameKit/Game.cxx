#include "pch.hxx"
#include "Game.hxx"
#include "../AssetKit/AssetHandler.hxx"
#include "../EventKit/EventQueue.hxx"
#include "../GraphicsKit/GraphicsHandler.hxx"

namespace Cyanite::GameKit {
	Game::Game(HWND handle) {
		_handle = handle;
		_graphics = std::make_unique<GraphicsKit::GraphicsHandler>(handle);
	}
	
	auto Game::StartUp() -> void {
		AssetKit::AssetHandler::Initialize();
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

	auto Game::OnResize(uint32_t width, uint32_t height) -> void {
		EventKit::EventQueue::Push(
			EventKit::Event{
				EventKit::EventType::WindowResize,
				nullptr,
				MathKit::Types::Vector2<uint32_t> { width, height},
			}
		);
	}
}

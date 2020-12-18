#include "pch.hxx"
#include "Engine.hxx"

#include "pch.hxx"
#include "Engine.hxx"
#include "../AssetKit/AssetHandler.hxx"
#include "../GraphicsKit/GraphicsHandler.hxx"

namespace Cyanite::GameKit {
	Engine::Engine(HWND handle) {
		_handle = handle;
		_graphics = std::make_unique<GraphicsKit::GraphicsHandler>(handle);
	}

	auto Engine::StartUp() -> void {
		AssetKit::AssetHandler::Initialize();
		_graphics->Initialize();
	}

	auto Engine::CleanUp() -> void {
		_graphics->Deinitialize();
	}

	auto Engine::Shutdown() -> void {
		_graphics->Deinitialize();
	}

	auto Engine::OnInit() -> void {
	}

	auto Engine::OnUpdate() -> void {
		_graphics->Update();
	}

	auto Engine::OnRender() -> void {
		_graphics->Render();
	}

	auto Engine::OnDestroy() -> void {
		this->Shutdown();
	}

	auto Engine::OnResize(uint32_t width, uint32_t height) -> void {
		_graphics->Resize(width, height);
	}
}

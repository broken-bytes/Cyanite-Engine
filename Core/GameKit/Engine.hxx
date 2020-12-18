#pragma once
#include "pch.hxx"
#include <memory>

#include "IEngine.hxx"
#include "../GraphicsKit/GraphicsHandler.hxx"

typedef void* HANDLE;

namespace Cyanite::GameKit {
	class Engine final : public IEngine {
	public:
		explicit Engine(HWND handle);
		~Engine() = default;
		auto StartUp() -> void override;
		auto CleanUp() -> void override;
		auto Shutdown() -> void override;

		auto OnInit() -> void override;
		auto OnUpdate() -> void override;
		auto OnRender() -> void override;
		auto OnDestroy() -> void override;
		auto OnResize(uint32_t width, uint32_t height) -> void override;
	private:
		HWND _handle;
		std::unique_ptr<GraphicsKit::GraphicsHandler> _graphics;		
	};
}


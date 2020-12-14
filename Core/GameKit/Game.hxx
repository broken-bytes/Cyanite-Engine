#pragma once
#include "pch.hxx"
#include <memory>
#include "../GraphicsKit/GraphicsHandler.hxx"

typedef void* HANDLE;

namespace Cyanide::GameKit {
	class IGame
	{
	public:
		virtual ~IGame() = default;
		virtual auto StartUp()   -> void = 0;
		virtual auto CleanUp()   -> void = 0;
		virtual auto Shutdown()  -> void = 0;

		virtual auto OnInit()    -> void = 0;
		virtual auto OnUpdate()  -> void = 0;
		virtual auto OnRender()  -> void = 0;
		virtual auto OnDestroy() -> void = 0;
	};


	class Game final : public IGame {
	public:
		explicit Game(HWND handle);
		~Game() = default;
		auto StartUp() -> void override;
		auto CleanUp() -> void override;
		auto Shutdown() -> void override;

		auto OnInit() -> void override;
		auto OnUpdate() -> void override;
		auto OnRender() -> void override;
		auto OnDestroy() -> void override;
	private:
		HWND _handle;
		std::unique_ptr<GraphicsKit::GraphicsHandler> _graphics;
		
	};
}


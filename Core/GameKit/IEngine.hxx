#pragma once

namespace Cyanite::GameKit {
	class IEngine
	{
	public:
		virtual ~IEngine() = default;
		virtual auto StartUp()   -> void = 0;
		virtual auto CleanUp()   -> void = 0;
		virtual auto Shutdown()  -> void = 0;

		virtual auto OnInit()    -> void = 0;
		virtual auto OnUpdate()  -> void = 0;
		virtual auto OnRender()  -> void = 0;
		virtual auto OnDestroy() -> void = 0;
		virtual auto OnResize(uint32_t width, uint32_t height)  -> void = 0;
	};
}

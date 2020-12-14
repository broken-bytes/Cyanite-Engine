#pragma once

#include "XInput.hxx"
#include "interfaces/IRumbleController.hxx"


namespace BrokenBytes::ControllerKit::Internal {
	class XInputController : IRumbleController {
	public:
		XInputController(XInputDevice gamepad);
		bool operator==(XInputDevice device) const;
		auto SetRumble(Types::Rumble motor, uint8_t strength) -> void override;
	private:
		XInputDevice _gamepad;
	};
}

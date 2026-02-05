#pragma once


//= new FunctionKeyBinding<Vec3>{ { SDL_SCANCODE_W, SDL_SCANCODE_LCTRL }, BINDFUNCTION<&InputManager::debugTapInputTranslation>(this), Vec3(0,1,0) };

class InputCreatorManager {
public:
	static int getInputState(std::string input);


};
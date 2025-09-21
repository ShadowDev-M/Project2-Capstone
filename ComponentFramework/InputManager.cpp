#include "InputManager.h"

bool PoolBindObject::call() {
	//Normally i'd place this code within the internal structure, however, due to compiling stuff, InputManager can't be accessed if the struct has <typename> above it and i don't want to overcomplicate
	if (bindingPtr) {
		for (auto& keybind : bindingPtr->keyb) {
			InputManager::getInstance();
			//return false if keybind isn't active
			if (!InputManager::getInstance().getKeyboardMap()->isActive(keybind)) {
				return false;
			}
		}
	}

	bindingPtr->call();
	return true;
}
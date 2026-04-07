#include "pch.h"
#include "InputManager.h"
#include "EditorManager.h"
#include "ScreenManager.h"
#include "Renderer.h"

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

InputManager::InputManager()
{
	pool.bindingPool.push_back(PoolBindObject());

	pool.bindingPool[0].bindingPtr = new FunctionKeyBinding<bool>{ { SDL_SCANCODE_ESCAPE }, BINDFUNCTION<&InputManager::debugClearDebugSelected>(this), true };
}

void InputManager::update(float deltaTime)
{
	SceneGraph& sceneGraph = SceneGraph::getInstance();

	mouse.update(deltaTime);
	keyboard.update(deltaTime);
	gamepad.Update(deltaTime);

#ifdef ENGINE_EDITOR
	if (GetIO().WantCaptureKeyboard && !windowFocused && !gameInputActive) {
		return;
	}
#endif

	for (auto& obj : pool.bindingPool) {
		obj.bindingPtr->call();
	}
}

bool InputManager::debugClearDebugSelected(std::pair<KeyBinding, std::tuple<bool>> input, SceneGraph* sceneGraph)
{
	bool condition = std::get<0>(input.second);

	if (keyboard.isPressed(input.first[0])) {
		SceneGraph::getInstance().debugSelectedAssets.clear();
		EditorManager::getInstance().ClearSelectedAsset();
		return true;
	}
	return false;
}

SDL_GameController* gamepadInputMap::findController()
{
	for (int i = 0; i < SDL_NumJoysticks(); i++) {
		if (SDL_IsGameController(i)) {
			return SDL_GameControllerOpen(i);
		}
	}

	return nullptr;
}

float gamepadInputMap::applyDeadzone(float value)
{
	if (std::abs(value) < deadzone) {
		return 0.0f;
	}

	// flip sign
	float sign = (value > 0) ? 1.0f : -1.0f;
	return sign * ((std::abs(value) - deadzone) / (1.0f - deadzone));
}

void gamepadInputMap::HandleEvents(const SDL_Event& event)
{
	switch (event.type) {
		// controller plugged in after engine runs
	case SDL_CONTROLLERDEVICEADDED:
		if (!controller) {
			controller = SDL_GameControllerOpen(event.cdevice.which);

			if (controller) {
				//std::cout << "Controller connected: " << SDL_GameControllerName(controller) << std::endl;
			}
		}
		break;

		// controller removed after engine runs
	case SDL_CONTROLLERDEVICEREMOVED:
		if (controller && event.cdevice.which == SDL_JoystickInstanceID(
			SDL_GameControllerGetJoystick(controller))) {
			SDL_GameControllerClose(controller);
			controller = findController();

			// resetting button states
			leftStickX = 0.0f;
			leftStickY = 0.0f;
			leftShoulderPressed = false;
			rightShoulderPressed = false;

		}
		break;

		// button presses
	case SDL_CONTROLLERBUTTONDOWN:
		if (isConnected()) {
			if (event.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
				leftShoulderPressed = true;
			}
			else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
				rightShoulderPressed = true;
			}
		}
		break;

	case SDL_CONTROLLERBUTTONUP:
		if (isConnected()) {
			if (event.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
				leftShoulderPressed = false;
			}
			else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
				rightShoulderPressed = false;
			}
		}
		break;

		// joystick
	case SDL_CONTROLLERAXISMOTION:
		if (isConnected()) {
			HandleAxisMotion(event.caxis.axis, event.caxis.value);
		}
		break;
	}
}

void gamepadInputMap::HandleAxisMotion(Uint8 axis, Sint16 value)
{
	// sdl returns values from -32768 to 32767, so gotta normalize
	float normValue = value / 32767.0f;

	switch (axis) {
	case SDL_CONTROLLER_AXIS_LEFTX:
		leftStickX = applyDeadzone(normValue);
		break;
	case SDL_CONTROLLER_AXIS_LEFTY:
		leftStickY = applyDeadzone(normValue);
		break;
	}
}

void gamepadInputMap::Update(float deltaTime)
{
	SceneGraph& sceneGraph = SceneGraph::getInstance();

	if (!isConnected()) return;
	if (sceneGraph.debugSelectedAssets.empty()) return;

	// function will move a selected actor based on joystick movement and shoulder button input
	// move values are holders for the actual inputs
	float moveX = leftStickX;
	float moveY = leftStickY;
	float moveZ = 0.0f;

	if (rightShoulderPressed) moveZ = 1.0f;
	if (leftShoulderPressed) moveZ = -1.0f;

	// making sure there is input
	if (std::abs(moveX) > 0.01f || std::abs(moveY) > 0.01f || std::abs(moveZ) > 0.01f) {
		float moveSpeed = 15.0f * deltaTime;

		// moving transform based on controller inputs
		for (const auto& actor : sceneGraph.debugSelectedAssets) {
			Ref<TransformComponent> transform = actor.second->GetComponent<TransformComponent>();

			if (transform) {
				Vec3 currentPos = transform->GetPosition();

				// have to flip y because of SDL
				transform->SetPos(currentPos.x + (moveX * moveSpeed), currentPos.y + (-moveY * moveSpeed), currentPos.z + (moveZ * moveSpeed));
			}
		}
	}
}

void keyboardInputMap::update(const float deltaTime)
{
	for (auto& keyCode : keyStates) {

		//If pressed, do behavior and then promote it to being held
		if (keyCode.second == InputState::Pressed) {
			//TODO: Pressed behavior
			//std::cout << keyCode.first << " Promoted to HELD" << std::endl;
			toggleKeyHeld(keyCode.first);
		}

	}

	//Max number of keys supported by your current keyboard
	int numKeys = 0;
	//array of keys determined by current keyboard size
	const Uint8* keys = SDL_GetKeyboardState(&numKeys);

	//update keychanges
	for (int i = 0; i < numKeys; i++) {

		//behavior when key is pressed
		if (keys[i] && isReleased(i)) {
			SDL_Scancode sc = static_cast<SDL_Scancode>(i);

			//std::cout << "Key pressed: " << SDL_GetScancodeName(sc) << std::endl;
			toggleKeyPress(i);

		}
		else if (!keys[i] && !isReleased(i)) {
			SDL_Scancode sc = static_cast<SDL_Scancode>(i);

			//std::cout << "Key released: " << SDL_GetScancodeName(sc) << std::endl;

			toggleKeyRelease(i);

		}
	}
}

void mouseInputMap::HandleEvents(const SDL_Event& sdlEvent, SceneGraph* sceneGraph)
{
	bool activeWindowHovered;

#ifdef ENGINE_EDITOR
	if (gameHovered) activeWindowHovered = gameHovered;
	else activeWindowHovered = sceneHovered;
#else
	activeWindowHovered = true;
#endif

	//Just checking if the title bar is clicked, just for convenience.
	if ((!activeWindowHovered) || (sdlEvent.motion.y >= scenePos.y && sdlEvent.motion.y <= (scenePos.y + sceneFrame))) {
		return;
	}

	static int lastX = 0, lastY = 0;
	const Uint8* keys = SDL_GetKeyboardState(NULL);

	static bool mouseHeld = false;

	switch (sdlEvent.type) {
	case SDL_MOUSEBUTTONDOWN:
		// allows imguizmo to handle mouse movement when editing gizmos
		if (ImGuizmo::IsOver() && ImGuizmo::IsUsing()) {
			return;
		}

		toggleKeyPress(sdlEvent.button.button);
		//std::cout << static_cast<int>(sdlEvent.button.button) << " is Pressed!" << std::endl;

		lastX = sdlEvent.button.x;
		lastY = sdlEvent.button.y;

		break;
	case SDL_MOUSEBUTTONUP:

		toggleKeyRelease(sdlEvent.button.button);
		//std::cout << static_cast<int>(sdlEvent.button.button) << " is Released!" << std::endl;

		// allows imguizmo to handle mouse movement when editing gizmos
		if (ImGuizmo::IsOver() && ImGuizmo::IsUsing()) {

			return;
		}


		lastX = sdlEvent.button.x;
		lastY = sdlEvent.button.y;

		if (sdlEvent.button.button == SDL_BUTTON_RIGHT) {
			mouseHeld = false;
		}
		if (sdlEvent.button.button == SDL_BUTTON_LEFT) {
#ifdef ENGINE_EDITOR
			if (sceneHovered) {
				//prepare for unintelligible logic for selecting 
				Ref<Actor> raycastedActor = Renderer::getInstance().PickActor(sdlEvent.button.x, sdlEvent.button.y);

				//an object was clicked
				if (raycastedActor) {
					if (!keys[SDL_SCANCODE_LCTRL] && !(sceneGraph->debugSelectedAssets.find(raycastedActor->getId()) != sceneGraph->debugSelectedAssets.end())) { sceneGraph->debugSelectedAssets.clear(); }

					if (sceneGraph->debugSelectedAssets.find(raycastedActor->getId()) != sceneGraph->debugSelectedAssets.end() && keys[SDL_SCANCODE_LCTRL]) { sceneGraph->debugSelectedAssets.erase(raycastedActor->getId()); }

					else {
						sceneGraph->debugSelectedAssets.emplace(raycastedActor->getId(), raycastedActor);
						EditorManager::getInstance().SetLastSelected(raycastedActor->getId());
					}
				}
				//no object was clicked, and left control isn't pressed (making sure the user didn't accidentally misclicked during multi object selection before clearing selection)
				else if (!keys[SDL_SCANCODE_LCTRL]) {
					sceneGraph->debugSelectedAssets.clear();
					EditorManager::getInstance().ClearSelectedAsset();
				}
			}
#endif
			mouseHeld = false;
		}

		break;
	case SDL_MOUSEMOTION:
		if (isHeld(SDL_BUTTON_LEFT) || isPressed(SDL_BUTTON_LEFT)) {
			// allows imguizmo to handle mouse movement when editing gizmos
			if (ImGuizmo::IsOver() && ImGuizmo::IsUsing()) {
				return;
			}

			// makes it so ImGui handles the mouse motion
			if (!sceneHovered) {
				toggleKeyPress(sdlEvent.button.button);
				return;
			}
		}

		break;
	default:
		break;
	}
}

void mouseInputMap::update(const float deltaTime)
{
	for (auto& keyCode : keyStates) {

		//If pressed, do behavior and then promote it to being held
		if (keyCode.second == InputState::Pressed) {
			//std::cout << keyCode.first << " Promoted to HELD" << std::endl;
			toggleKeyHeld(keyCode.first);
		}

	}
}

std::vector<PoolBindObject*> keyBindingObjectPool::getBindings(KeyBinding keybinding)
{
	std::vector<PoolBindObject*> bindings;

	//go through the pool and push any objects that use the keybinding
	for (PoolBindObject obj : bindingPool) {
		if (obj.bindingPtr) {
			if (keybinding == obj.bindingPtr->keyb) {
				bindings.push_back(&obj);
			}
		}
	}

	return bindings;
}

void keyBindingObjectPool::call(KeyBinding keybinding)
{
	//go through the pool and call any objects that use the keybinding
	for (PoolBindObject obj : bindingPool) {
		if (obj.bindingPtr) {
			if (keybinding == obj.bindingPtr->keyb) {
				obj.bindingPtr->call();
			}
		}
	}
}


template<typename ...Args>
inline void FunctionKeyBinding<Args...>::call()
{
	for (auto& keypress : keyb) {

	}

	function({ keyb, reqArguments }, &SceneGraph::getInstance());
}

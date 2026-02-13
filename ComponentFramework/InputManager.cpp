#include "pch.h"
#include "InputManager.h"
#include "EditorManager.h"

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
	pool.bindingPool.push_back(PoolBindObject());
	pool.bindingPool.push_back(PoolBindObject());
	pool.bindingPool.push_back(PoolBindObject());
	/*pool.bindingPool.push_back(PoolBindObject());
	pool.bindingPool.push_back(PoolBindObject());
	pool.bindingPool.push_back(PoolBindObject());
	pool.bindingPool.push_back(PoolBindObject());
	pool.bindingPool.push_back(PoolBindObject());*/

	//FunctionKeyBinding<Vec3> test = { { SDL_SCANCODE_W, SDL_SCANCODE_LCTRL }, BINDFUNCTION<&InputManager::debugTapInputTranslation>(this), Vec3(0,1,0) };

	pool.bindingPool[0].bindingPtr = new FunctionKeyBinding<Vec3>{ { SDL_SCANCODE_W, SDL_SCANCODE_LCTRL }, BINDFUNCTION<&InputManager::debugTapInputTranslation>(this), Vec3(0,1,0) };
	pool.bindingPool[1].bindingPtr = new FunctionKeyBinding<Vec3>{ { SDL_SCANCODE_S, SDL_SCANCODE_LCTRL }, BINDFUNCTION<&InputManager::debugTapInputTranslation>(this), Vec3(0,-1,0) };
	pool.bindingPool[2].bindingPtr = new FunctionKeyBinding<Vec3>{ { SDL_SCANCODE_A, SDL_SCANCODE_LCTRL }, BINDFUNCTION<&InputManager::debugTapInputTranslation>(this), Vec3(-1,0,0) };
	pool.bindingPool[3].bindingPtr = new FunctionKeyBinding<Vec3>{ { SDL_SCANCODE_D, SDL_SCANCODE_LCTRL }, BINDFUNCTION<&InputManager::debugTapInputTranslation>(this), Vec3(1,0,0) };

	/*pool.bindingPool[4].bindingPtr = new FunctionKeyBinding<Vec3>{ { SDL_SCANCODE_W }, BINDFUNCTION<&InputManager::debugCamInputTranslation>(this), Vec3(0,1,0) };
	pool.bindingPool[5].bindingPtr = new FunctionKeyBinding<Vec3>{ { SDL_SCANCODE_S }, BINDFUNCTION<&InputManager::debugCamInputTranslation>(this), Vec3(0,-1,0) };
	pool.bindingPool[6].bindingPtr = new FunctionKeyBinding<Vec3>{ { SDL_SCANCODE_A }, BINDFUNCTION<&InputManager::debugCamInputTranslation>(this), Vec3(-1,0,0) };
	pool.bindingPool[7].bindingPtr = new FunctionKeyBinding<Vec3>{ { SDL_SCANCODE_D }, BINDFUNCTION<&InputManager::debugCamInputTranslation>(this), Vec3(1,0,0) };*/

	//pool.bindingPool[4].bindingPtr = new FunctionKeyBinding<bool>{ { SDL_SCANCODE_SPACE }, BINDFUNCTION<&InputManager::startGame>(this), true };


	/*debugCamInputTranslation({
				{{SDL_SCANCODE_W}, Vec3(0, 1, 0)},
				{{SDL_SCANCODE_S}, Vec3(0, -1, 0)},
				{{SDL_SCANCODE_A}, Vec3(-1, 0, 0) },
				{{SDL_SCANCODE_D}, Vec3(1, 0, 0)}
		}, sceneGraph);*/

}

void InputManager::update(float deltaTime, SceneGraph* sceneGraph)
{
	sceneGraph->checkValidCamera();
	mouse.update(deltaTime);

	if (GetIO().WantCaptureKeyboard && !dockingFocused) {
		return;
	}

	//update keyboard object
	keyboard.update(deltaTime);

	gamepad.Update(deltaTime, sceneGraph);

	//Check which scene debug vs playing
	if (true) { //temp set to always true until we can define debug vs playable scenes


		// IDEA:
		//Store a pool of threads for each function, and then 



		/*test.function = std::bind(&InputManager::debugTapInputTranslation, this,
			std::placeholders::_1, std::placeholders::_2);*/

			/*FunctionKeyBinding<Vec3> test = {{ SDL_SCANCODE_W, SDL_SCANCODE_LCTRL }, BINDFUNCTION<&InputManager::debugTapInputTranslation>(this), Vec3(0,1,0)};

			test.call();*/

		for (auto& obj : pool.bindingPool) {
			obj.bindingPtr->call();
		}

		//pool.bindingPool.push_back(PoolBindObject());
		/*test.keyb = { SDL_SCANCODE_W, SDL_SCANCODE_LCTRL };
		test.function = BINDFUNCTION<&InputManager::debugTapInputTranslation>(this);*/

		//pool.bindingPool.push_back(&test);



		/*test.function({
			{{SDL_SCANCODE_W, SDL_SCANCODE_LCTRL }, Vec3(0, 1, 0)},
			{{SDL_SCANCODE_S, SDL_SCANCODE_LCTRL }, Vec3(0, -1, 0)},
			{{SDL_SCANCODE_A, SDL_SCANCODE_LCTRL }, Vec3(-1, 0, 0)},
			{{SDL_SCANCODE_D, SDL_SCANCODE_LCTRL }, Vec3(1, 0, 0)}
			}, sceneGraph);*/


			/*if (!test.function({{SDL_SCANCODE_W, SDL_SCANCODE_LCTRL }, Vec3(0, 1, 0)}, sceneGraph) &&
				!test.function({{SDL_SCANCODE_S, SDL_SCANCODE_LCTRL }, Vec3(0, -1, 0)}, sceneGraph) &&
				!test.function({{SDL_SCANCODE_A, SDL_SCANCODE_LCTRL }, Vec3(-1, 0, 0)}, sceneGraph) &&
				!test.function({{SDL_SCANCODE_D, SDL_SCANCODE_LCTRL }, Vec3(1, 0, 0)}, sceneGraph)
			) {

				debugCamInputTranslation({
					{{SDL_SCANCODE_W}, Vec3(0, 1, 0)},
					{{SDL_SCANCODE_S}, Vec3(0, -1, 0)},
					{{SDL_SCANCODE_A}, Vec3(-1, 0, 0) },
					{{SDL_SCANCODE_D}, Vec3(1, 0, 0)}
					}, sceneGraph);
			}*/

			////bind keypress to camera and test for swap
			//debugInputCamSwap({
			//	{SDL_SCANCODE_X, sceneGraph->GetActor("cameraActor")->GetComponent<CameraComponent>()},
			//	{SDL_SCANCODE_C, sceneGraph->GetActor("cameraActor2")->GetComponent<CameraComponent>()}
			//	}, sceneGraph);

	}


}

bool InputManager::startGame(std::pair<KeyBinding, std::tuple<bool>> input, SceneGraph* sceneGraph)
{
	//key is pressed
	if (keyboard.isPressed(input.first[0])) {

		//start game
		//sceneGraph->Start();
		return true;
	}
	return false;
}

void InputManager::debugInputCamSwap(std::vector<std::pair<SDL_Scancode, Ref<CameraComponent>>> inputMap, SceneGraph* sceneGraph)
{
	for (auto& keyPress : inputMap) {
		if (keyPress.second) {
			//key is pressed
			if (keyboard.isPressed(keyPress.first)) {

				//set active camera to keypress's binded camera
				if (keyPress.second) {
					sceneGraph->setUsedCamera(keyPress.second);
				}
			}
		}
	}
}

bool InputManager::debugTapInputTranslation(std::pair<KeyBinding, std::tuple<Vec3>> inputMap, SceneGraph* sceneGraph)
{
	Ref<Actor> camera = (sceneGraph->getUsedCamera()->GetUserActor());


	bool hasMoved = false;
	if (camera) {
		Vec3 keyPressVector = std::get<0>(inputMap.second);


		bool binding_condition_failed = false;

		//First in the binding should be tapped not held
		if (!keyboard.isPressed(inputMap.first[0])) {
			return 0;
		}

		//Check if other keys are active (first one just checked beforehand to give the tap effect, while other keycodes in binding act as 'requirements' such as CTRL)
		for (SDL_Scancode& keyBind : inputMap.first) {
			//if any of the keys in the binding are not pressed, then binding condition is not met, so move on to the next binding test
			if (!keyboard.isActive(keyBind)) binding_condition_failed = true;
		}

		if (binding_condition_failed) return 0;




		//Put a slider here for stud based movement
		Vec3 inputVector = keyPressVector * studMultiplier; //<- slider multiplier here

		Quaternion q = camera->GetComponent<TransformComponent>()->GetOrientation();

		Quaternion rotation = (QMath::normalize(q));
		//	camera->GetComponent<TransformComponent>()->GetPosition().print();

			//convert local direction into world coords 
		Vec3 worldForward = QMath::rotate(inputVector, rotation);

		if (!(sceneGraph->debugSelectedAssets.empty())) {
			//auto& debugGraph = sceneGraph.debugSelectedAssets;


			for (const auto& obj : sceneGraph->debugSelectedAssets) {

				obj.second->GetComponent<TransformComponent>()->SetTransform(
					obj.second->GetComponent<TransformComponent>()->GetPosition() + worldForward,
					obj.second->GetComponent<TransformComponent>()->GetOrientation(),
					obj.second->GetComponent<TransformComponent>()->GetScale()
				);
				hasMoved = true;

			}
		}

	}
	return hasMoved;
}

bool InputManager::debugCamInputTranslation(std::pair<KeyBinding, std::tuple<Vec3>> inputMap, SceneGraph* sceneGraph)
{

	Ref<Actor> camera = (sceneGraph->getUsedCamera()->GetUserActor());

	Vec3 keyPressVector = std::get<0>(inputMap.second);

	if (camera) {
		if (!keyboard.isPressed(inputMap.first[0]) || keyboard.isActive(SDL_SCANCODE_LCTRL)) return false;

		//Put a slider here for stud based movement
		Vec3 inputVector = keyPressVector * studMultiplier; //<- slider multiplier here

		Quaternion q = camera->GetComponent<TransformComponent>()->GetOrientation();

		Quaternion rotation = (QMath::normalize(q));
		//	camera->GetComponent<TransformComponent>()->GetPosition().print();

			//convert local direction into world coords 
		Vec3 worldForward = QMath::rotate(inputVector, rotation);


		//no condition needed in this case
		camera->GetComponent<TransformComponent>()->SetTransform(
			camera->GetComponent<TransformComponent>()->GetPosition() + worldForward,
			q
		);
		camera->GetComponent<CameraComponent>()->fixCameraToTransform();

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
				std::cout << "Controller connected: " << SDL_GameControllerName(controller) << std::endl;
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

void gamepadInputMap::Update(float deltaTime, SceneGraph* sceneGraph_)
{
	if (!isConnected()) return;
	if (sceneGraph_->debugSelectedAssets.empty()) return;

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
		for (const auto& actor : sceneGraph_->debugSelectedAssets) {
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
			std::cout << keyCode.first << " Promoted to HELD" << std::endl;
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

			std::cout << "Key pressed: " << SDL_GetScancodeName(sc) << std::endl;
			toggleKeyPress(i);

		}
		else if (!keys[i] && !isReleased(i)) {
			SDL_Scancode sc = static_cast<SDL_Scancode>(i);

			std::cout << "Key released: " << SDL_GetScancodeName(sc) << std::endl;

			toggleKeyRelease(i);

		}

	}
}

void mouseInputMap::HandleEvents(const SDL_Event& sdlEvent, SceneGraph* sceneGraph)
{
	//std::cout << dockingClicked << std

	//Just checking if the title bar is clicked, just for convenience.
	if ((!dockingHovered) || (sdlEvent.motion.y >= dockingPos.y && sdlEvent.motion.y <= (dockingPos.y + frameHeight))) {

		//if (dockingClicked) { std::cout << "eee \n"; }


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
		std::cout << static_cast<int>(sdlEvent.button.button) << " is Pressed!" << std::endl;

		lastX = sdlEvent.button.x;
		lastY = sdlEvent.button.y;




		break;
	case SDL_MOUSEBUTTONUP:


		toggleKeyRelease(sdlEvent.button.button);
		std::cout << static_cast<int>(sdlEvent.button.button) << " is Released!" << std::endl;

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

			// makes it so ImGui handles the mouse click
			if (!dockingHovered) {
				mouseHeld = false;
				return;
			}

			lastX = sdlEvent.button.x;
			lastY = sdlEvent.button.y;

			//Ref<Actor> cameraActor_ = std::dynamic_pointer_cast<Actor>(sceneGraph->getUsedCamera()->GetUserActor());

			//Vec3 startPos = cameraActor_->GetComponent<TransformComponent>()->GetPosition();
			//Vec3 endPos = startPos + Raycast::screenRayCast(lastX, lastY, sceneGraph->getUsedCamera()->GetProjectionMatrix(), sceneGraph->getUsedCamera()->GetViewMatrix());

			/*if (sdlEvent.type == SDL_MOUSEBUTTONDOWN) {
				Ref<Actor> pickobj = SceneGraph::getInstance().pickColour(sdlEvent.button.x, sdlEvent.button.y);

				if (pickobj)pickobj->ListComponents();
			}*/

			//prepare for unintelligible logic for selecting 
			Ref<Actor> raycastedActor = sceneGraph->pickColour(sdlEvent.button.x, sdlEvent.button.y);

			//if (!raycastedActor) raycastedActor = collisionSystem->PhysicsRaycast(startPos, endPos);

			//(Slightly) more expensive debug selector if nothing was selected with the ColliderComponent PhysicsRaycast
			//if (!raycastedActor) raycastedActor = sceneGraph->MeshRaycast(startPos, endPos);

			//an object was clicked
			if (raycastedActor) {


				if (!keys[SDL_SCANCODE_LCTRL] && !(sceneGraph->debugSelectedAssets.find(raycastedActor->getId()) != sceneGraph->debugSelectedAssets.end())) { sceneGraph->debugSelectedAssets.clear(); }

				if (sceneGraph->debugSelectedAssets.find(raycastedActor->getId()) != sceneGraph->debugSelectedAssets.end() && keys[SDL_SCANCODE_LCTRL]) { sceneGraph->debugSelectedAssets.erase(raycastedActor->getId()); }

				else sceneGraph->debugSelectedAssets.emplace(raycastedActor->getId(), raycastedActor);

			}
			//no object was clicked, and left control isn't pressed (making sure the user didn't accidentally misclicked during multi object selection before clearing selection)
			else if (!keys[SDL_SCANCODE_LCTRL])sceneGraph->debugSelectedAssets.clear();



			//startPos.print();
			//endPos.print();
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
			if (!dockingHovered) {
				toggleKeyPress(sdlEvent.button.button);
				return;
			}

			float deltaX = sdlEvent.motion.x - (float)lastX;
			float deltaY = sdlEvent.motion.y - (float)lastY;
			lastX = sdlEvent.motion.x;
			lastY = sdlEvent.motion.y;

			Ref<Actor> camera = std::dynamic_pointer_cast<Actor>(sceneGraph->getUsedCamera()->GetUserActor());


			if (!(sceneGraph->debugSelectedAssets.empty())) {
				//get direction vector of new vector of movement from old mouse pos and new mouse pos



				auto& debugGraph = sceneGraph->debugSelectedAssets;

				float w, h;

				w = (float) SceneGraph::SCENEWIDTH;
				h = (float) SceneGraph::SCENEHEIGHT;

				//SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &w, &h);

				float aspectRatio = static_cast<float>(w) / static_cast<float>(h);


				ImVec2 scaledTexture;

				// Calculate scaled dimensions based on aspect ratio
				if (dockingSize.x / aspectRatio <= dockingSize.y)
				{
					scaledTexture.x = dockingSize.x;
					scaledTexture.y = dockingSize.x / aspectRatio;
				}
				else
				{
					scaledTexture.y = dockingSize.y;
					scaledTexture.x = dockingSize.y * aspectRatio;
				}


				for (const auto& obj : debugGraph) {
					Ref<TransformComponent> transform = obj.second->GetComponent<TransformComponent>();
					Vec3 vectorMove = transform->GetPosition() + Vec3(deltaX, -deltaY, transform->GetPosition().z) * (camera->GetComponent<TransformComponent>()->GetPosition().z - transform->GetPosition().z) / 40.0f * 0.045f * (h / scaledTexture.y);





					transform->SetPos(vectorMove.x, vectorMove.y, transform->GetPosition().z);

				}


			}
		}

		if (isHeld(SDL_BUTTON_RIGHT) || isPressed(SDL_BUTTON_RIGHT)) {

			// makes it so ImGui handles the mouse motion
			if (!dockingHovered) {
				toggleKeyPress(sdlEvent.button.button);
				return;
			}

			float deltaX = sdlEvent.motion.x - (float)lastX;
			float  deltaY = sdlEvent.motion.y - (float)lastY;
			lastX = sdlEvent.motion.x;
			lastY = sdlEvent.motion.y;

			Ref<Actor> camera = std::dynamic_pointer_cast<Actor>(sceneGraph->getUsedCamera()->GetUserActor());
			Ref<TransformComponent> transform = camera->GetComponent<TransformComponent>();

			float speedDrag = 1.0f;

			Vec3 vectorMove = transform->GetPosition() + Vec3(deltaX, -deltaY, transform->GetPosition().z) * -speedDrag * 0.045f;
			transform->SetPos(vectorMove.x, vectorMove.y, transform->GetPosition().z);

			if (camera->GetComponent<CameraComponent>()) {
				camera->GetComponent<CameraComponent>()->fixCameraToTransform();
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
			//TODO: Pressed behavior




			std::cout << keyCode.first << " Promoted to HELD" << std::endl;
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

#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <map>
#include "Debug.h"
#include "SceneGraph.h"
#include "TransformComponent.h"
#include <memory>
#include "CameraActor.h"
#include "imgui.h"
#include "CollisionSystem.h"
#include <functional>
#include <tuple>
#include <utility>
#include <thread>
#include <chrono>

using namespace ImGui;


enum class InputState {
	Released,
	Pressed,
	Held // For continuous input checks
};

typedef std::vector<SDL_Scancode> KeyBinding;


template <auto FUNCTION, typename T>
auto BINDFUNCTION(T* obj) {
	return std::bind(FUNCTION, obj, std::placeholders::_1, std::placeholders::_2);
}


///Base form of functionKeyBinding without the function (The function size is dependant on arguments, so this should be used as a wrapper)
struct FunctionKeyBindingWrapper {
	
	KeyBinding keyb;

	virtual ~FunctionKeyBindingWrapper() = default;


	virtual void call() = 0;

};

///Structure to store a keybinding to a function
template<typename... Args>
struct FunctionKeyBinding : FunctionKeyBindingWrapper {
public:

	KeyBinding keyb;

	//storage for internal function 
	std::function<bool(std::pair<KeyBinding, std::tuple<Args...>>, SceneGraph*)> function;
	
	//unique arguments for the internal function to be called
	std::tuple<Args...> reqArguments;

	//constructor
	FunctionKeyBinding(KeyBinding k, std::function<bool(std::pair<KeyBinding, std::tuple<Args...>>, SceneGraph*)> f, std::tuple<Args... > rA)
        : keyb(k), function(f), reqArguments(rA) {}
	
	/*FunctionKeyBinding(KeyBinding k, std::function<bool(KeyBinding, SceneGraph*)> f)
		: keyb(k), function(f) {
	}*/



	///Calls the function associated with the binding
	void call() override {
		for (auto& keypress : keyb) {

		}

		function({ keyb, reqArguments }, &SceneGraph::getInstance());
	};
};

///We can't store functionKeyBinding in a 
struct PoolBindObject {

	//ptr to object
	FunctionKeyBindingWrapper* bindingPtr;

	bool flagUsedThisFrame = false;

	bool call();

	//remove ptr
	void Reset() { delete bindingPtr; };

	//If item is destroyed, delete its ptr
	~PoolBindObject() { delete bindingPtr;}

};



///an object pool structure
struct keyBindingObjectPool {
	std::vector<PoolBindObject> bindingPool;
	
	///return all poolObjects that share the parametre's keybinding
	std::vector<PoolBindObject*> getBindings(KeyBinding keybinding) {

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

	///call all poolObjects that share the parametre's keybinding
	void call(KeyBinding keybinding) {

		//go through the pool and call any objects that use the keybinding
		for (PoolBindObject obj : bindingPool) {
			if (obj.bindingPtr) {
				if (keybinding == obj.bindingPtr->keyb) {
					obj.bindingPtr->call();
				}
			}
		}
	}

};


class InputMap {
protected:

	//mapping a keyCode with it's current state
	std::map<int, InputState> keyStates;
	
public:
	virtual bool isActive(int keyCode) {
		//Active == both pressed and held
		if (keyStates[keyCode] == InputState::Pressed) return true;

		return keyStates[keyCode] == InputState::Held;
	}

	virtual bool isPressed(int keyCode) {
		return keyStates[keyCode] == InputState::Pressed;
	}

	virtual bool isReleased(int keyCode) {
		return keyStates[keyCode] == InputState::Released;
	}

	virtual bool isHeld(int keyCode) {
		return keyStates[keyCode] == InputState::Held;
	}

	virtual void toggleKeyPress(int keyCode) {
		keyStates[keyCode] = InputState::Pressed;
	}

	virtual void toggleKeyRelease(int keyCode) {
		keyStates[keyCode] = InputState::Released;
	}

	virtual void toggleKeyHeld(int keyCode) {
		keyStates[keyCode] = InputState::Held;
	}

	virtual void update(const float deltaTime_) = 0;
};

class mouseInputMap : public InputMap {
private:


public:

	bool dockingHovered = 0;
	bool dockingClicked = 0;
	ImVec2 dockingPos;   // top-left corner
	ImVec2 dockingSize;  // width/height

	float frameHeight = 0;

	void HandleEvents(const SDL_Event& sdlEvent, SceneGraph* sceneGraph, CollisionSystem* collisionSystem) {

		//std::cout << dockingClicked << std

		//Just checking if the title bar is clicked, just for convenience.
		if (!dockingHovered || (sdlEvent.motion.y >= dockingPos.y && sdlEvent.motion.y <= (dockingPos.y + frameHeight))) {
			
			//if (dockingClicked) { std::cout << "eee \n"; }


			return;

		}
		static int lastX = 0, lastY = 0;
		const Uint8* keys = SDL_GetKeyboardState(NULL);



		static bool mouseHeld = false;

		switch (sdlEvent.type) {
		case SDL_MOUSEBUTTONDOWN:


			toggleKeyPress(sdlEvent.button.button);
			std::cout << static_cast<int>(sdlEvent.button.button) << " is Pressed!" << std::endl;

			lastX = sdlEvent.button.x;
			lastY = sdlEvent.button.y;




			break;
		case SDL_MOUSEBUTTONUP:
			
			toggleKeyRelease(sdlEvent.button.button);
			std::cout << static_cast<int>(sdlEvent.button.button) << " is Released!" << std::endl;

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

				Ref<Actor> cameraActor_ = std::dynamic_pointer_cast<Actor>(sceneGraph->getUsedCamera()->GetUserActor());

				Vec3 startPos = cameraActor_->GetComponent<TransformComponent>()->GetPosition();
				Vec3 endPos = startPos + Raycast::screenRayCast(lastX, lastY, sceneGraph->getUsedCamera()->GetProjectionMatrix(), sceneGraph->getUsedCamera()->GetViewMatrix());

				if (sdlEvent.type == SDL_MOUSEBUTTONDOWN) {
					Ref<Actor> pickobj = SceneGraph::getInstance().pickColour(sdlEvent.button.x, sdlEvent.button.y);

					if (pickobj)pickobj->ListComponents();
				}

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

				// makes it so ImGui handles the mouse motion
				if ( !dockingHovered) {
					toggleKeyPress(sdlEvent.button.button);
					return;
				}

				int deltaX = sdlEvent.motion.x - lastX;
				int deltaY = sdlEvent.motion.y - lastY;
				lastX = sdlEvent.motion.x;
				lastY = sdlEvent.motion.y;

				Ref<Actor> camera = std::dynamic_pointer_cast<Actor>(sceneGraph->getUsedCamera()->GetUserActor());


				if (!(sceneGraph->debugSelectedAssets.empty())) {
					//get direction vector of new vector of movement from old mouse pos and new mouse pos

					

					auto& debugGraph = sceneGraph->debugSelectedAssets;

					int w, h;

					w = SceneGraph::SCENEWIDTH;
					h = SceneGraph::SCENEHEIGHT;

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
						Vec3 vectorMove = transform->GetPosition() + Vec3(deltaX, -deltaY, transform->GetPosition().z) * (camera->GetComponent<TransformComponent>()->GetPosition().z - transform->GetPosition().z) / 40 * 0.045 * (  h / scaledTexture.y);





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

				int deltaX = sdlEvent.motion.x - lastX;
				int deltaY = sdlEvent.motion.y - lastY;
				lastX = sdlEvent.motion.x;
				lastY = sdlEvent.motion.y;

				Ref<Actor> camera = std::dynamic_pointer_cast<Actor>(sceneGraph->getUsedCamera()->GetUserActor());


					//std::cout << "trerst" << std::endl;
				Ref<TransformComponent> transform = camera->GetComponent<TransformComponent>();

				float speedDrag = 1;

				Vec3 vectorMove = transform->GetPosition() + Vec3(deltaX, -deltaY, transform->GetPosition().z) * -speedDrag * 0.045;
				transform->SetPos(vectorMove.x, vectorMove.y, transform->GetPosition().z);
				

			}
			break;
		default:
			break;
		}

	}

	void update(const float deltaTime) override {
		


		
		
		for (auto& keyCode : keyStates) {

			//If pressed, do behavior and then promote it to being held
			if (keyCode.second == InputState::Pressed) {
				//TODO: Pressed behavior




				std::cout << keyCode.first << " Promoted to HELD" << std::endl;
				toggleKeyHeld(keyCode.first);
			}

		}
	}
};


class keyboardInputMap : public InputMap {
private:

	
public:

	void update(const float deltaTime) override {
		
		
		
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

};

class InputManager
{
private:
	InputManager() {
		pool.bindingPool.push_back(PoolBindObject());
		pool.bindingPool.push_back(PoolBindObject());
		pool.bindingPool.push_back(PoolBindObject());
		pool.bindingPool.push_back(PoolBindObject());
		pool.bindingPool.push_back(PoolBindObject());
		pool.bindingPool.push_back(PoolBindObject());
		pool.bindingPool.push_back(PoolBindObject());
		pool.bindingPool.push_back(PoolBindObject());
		pool.bindingPool.push_back(PoolBindObject());

		//FunctionKeyBinding<Vec3> test = { { SDL_SCANCODE_W, SDL_SCANCODE_LCTRL }, BINDFUNCTION<&InputManager::debugTapInputTranslation>(this), Vec3(0,1,0) };

		pool.bindingPool[0].bindingPtr = new FunctionKeyBinding<Vec3> { { SDL_SCANCODE_W, SDL_SCANCODE_LCTRL }, BINDFUNCTION<&InputManager::debugTapInputTranslation>(this), Vec3(0,1,0) };
		pool.bindingPool[1].bindingPtr = new FunctionKeyBinding<Vec3>{ { SDL_SCANCODE_S, SDL_SCANCODE_LCTRL }, BINDFUNCTION<&InputManager::debugTapInputTranslation>(this), Vec3(0,-1,0) };
		pool.bindingPool[2].bindingPtr = new FunctionKeyBinding<Vec3>{ { SDL_SCANCODE_A, SDL_SCANCODE_LCTRL }, BINDFUNCTION<&InputManager::debugTapInputTranslation>(this), Vec3(-1,0,0) };
		pool.bindingPool[3].bindingPtr = new FunctionKeyBinding<Vec3>{ { SDL_SCANCODE_D, SDL_SCANCODE_LCTRL }, BINDFUNCTION<&InputManager::debugTapInputTranslation>(this), Vec3(1,0,0) };

		pool.bindingPool[4].bindingPtr = new FunctionKeyBinding<Vec3>{ { SDL_SCANCODE_W }, BINDFUNCTION<&InputManager::debugCamInputTranslation>(this), Vec3(0,1,0) };
		pool.bindingPool[5].bindingPtr = new FunctionKeyBinding<Vec3>{ { SDL_SCANCODE_S }, BINDFUNCTION<&InputManager::debugCamInputTranslation>(this), Vec3(0,-1,0) };
		pool.bindingPool[6].bindingPtr = new FunctionKeyBinding<Vec3>{ { SDL_SCANCODE_A }, BINDFUNCTION<&InputManager::debugCamInputTranslation>(this), Vec3(-1,0,0) };
		pool.bindingPool[7].bindingPtr = new FunctionKeyBinding<Vec3>{ { SDL_SCANCODE_D }, BINDFUNCTION<&InputManager::debugCamInputTranslation>(this), Vec3(1,0,0) };

		pool.bindingPool[8].bindingPtr = new FunctionKeyBinding<bool>{ { SDL_SCANCODE_SPACE }, BINDFUNCTION<&InputManager::startGame>(this), true};


		/*debugCamInputTranslation({
					{{SDL_SCANCODE_W}, Vec3(0, 1, 0)},
					{{SDL_SCANCODE_S}, Vec3(0, -1, 0)},
					{{SDL_SCANCODE_A}, Vec3(-1, 0, 0) },
					{{SDL_SCANCODE_D}, Vec3(1, 0, 0)}
			}, sceneGraph);*/


	}
	// delete copy and move constructers
	InputManager(const InputManager&) = delete;
	InputManager(InputManager&&) = delete;
	InputManager& operator = (const InputManager&) = delete;
	InputManager& operator = (InputManager&&) = delete;

	keyboardInputMap keyboard;
	mouseInputMap mouse;



	float studMultiplier = 0.5f;

	keyBindingObjectPool pool;

	bool dockingFocused = false;

public:
	
	keyboardInputMap* getKeyboardMap() { return &keyboard; }

	mouseInputMap* getMouseMap() { return &mouse; }

	void updateDockingFocused(bool state) { dockingFocused = state; }

	// Meyers Singleton (from JPs class)
	static InputManager& getInstance() {
		static InputManager instance;
		return instance;
	}

	
	void update(float deltaTime, SceneGraph* sceneGraph) {
		sceneGraph->checkValidCamera();
		mouse.update(deltaTime);

		if (GetIO().WantCaptureKeyboard && !dockingFocused) {
			return;
		}

		//update keyboard object
		keyboard.update(deltaTime);
		
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

	// getter and setter for stud multipler (used in slider)
	float GetStudMultiplier() { return studMultiplier; }
	void SetStudMultiplier(float studMulti_) { studMultiplier = studMulti_; }

	bool startGame(std::pair<KeyBinding, std::tuple<bool>> input, SceneGraph* sceneGraph) {
		
			//key is pressed
			if (keyboard.isPressed(input.first[0])) {

				//start game
				std::cout << "aear" << std::endl;
				sceneGraph->Start();
				return true;
			}
			return false;
	}

	void debugInputCamSwap(std::vector<std::pair<SDL_Scancode, Ref<CameraComponent>>> inputMap, SceneGraph* sceneGraph) {
		
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

	/// Allows for a KeyInput to be associated to a translation of a sceneGraph's debug selections
	bool debugTapInputTranslation(std::pair<KeyBinding, std::tuple<Vec3>> inputMap, SceneGraph* sceneGraph) {
		
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

				Quaternion q = camera->GetComponent<TransformComponent>()->GetQuaternion();

				Quaternion rotation = (QMath::normalize(q));
				//	camera->GetComponent<TransformComponent>()->GetPosition().print();

					//convert local direction into world coords 
				Vec3 worldForward = QMath::rotate(inputVector, rotation);

				if (!(sceneGraph->debugSelectedAssets.empty())) {
					//auto& debugGraph = sceneGraph.debugSelectedAssets;


					for (const auto& obj : sceneGraph->debugSelectedAssets) {

						obj.second->GetComponent<TransformComponent>()->SetTransform(
							obj.second->GetComponent<TransformComponent>()->GetPosition() + worldForward,
							obj.second->GetComponent<TransformComponent>()->GetQuaternion(),
							obj.second->GetComponent<TransformComponent>()->GetScale()
						);
						hasMoved = true;

					}
				}				
			
		}
		return hasMoved;
	}

	bool debugCamInputTranslation(std::pair<KeyBinding, std::tuple<Vec3>> inputMap, SceneGraph* sceneGraph) {

		Ref<Actor> camera = (sceneGraph->getUsedCamera()->GetUserActor());

		Vec3 keyPressVector = std::get<0>(inputMap.second);

		if (camera) {


					
				if (!keyboard.isPressed(inputMap.first[0]) || keyboard.isActive(SDL_SCANCODE_LCTRL)) return 0;




				//Put a slider here for stud based movement
				Vec3 inputVector = keyPressVector * studMultiplier; //<- slider multiplier here

				Quaternion q = camera->GetComponent<TransformComponent>()->GetQuaternion();

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

	void HandleEvents(const SDL_Event& sdlEvent, SceneGraph* sceneGraph, CollisionSystem* collisionSystem) {
		sceneGraph->checkValidCamera();

		mouse.HandleEvents(sdlEvent, sceneGraph, collisionSystem);

		
	}


	~InputManager() { }
};



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

using namespace ImGui;


enum class InputState {
	Released,
	Pressed,
	Held // For continuous input checks
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

	void HandleEvents(const SDL_Event& sdlEvent, SceneGraph* sceneGraph, CollisionSystem* collisionSystem) {
		if (GetIO().WantCaptureMouse) {
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

			if (sdlEvent.button.button == SDL_BUTTON_RIGHT) {
				mouseHeld = false;
			}
			if (sdlEvent.button.button == SDL_BUTTON_LEFT) {

				// makes it so ImGui handles the mouse click
				if (GetIO().WantCaptureMouse) {
					mouseHeld = false;
					return;
				}

				lastX = sdlEvent.button.x;
				lastY = sdlEvent.button.y;

				Ref<Actor> cameraActor_ = std::dynamic_pointer_cast<Actor>(sceneGraph->getUsedCamera()->GetUserActor());

				Vec3 startPos = cameraActor_->GetComponent<TransformComponent>()->GetPosition();
				Vec3 endPos = startPos + Raycast::screenRayCast(lastX, lastY, sceneGraph->getUsedCamera()->GetProjectionMatrix(), sceneGraph->getUsedCamera()->GetViewMatrix());


				//prepare for unintelligible logic for selecting 
				Ref<Actor> raycastedActor = collisionSystem->PhysicsRaycast(startPos, endPos);

				//(Slightly) more expensive debug selector if nothing was selected with the ColliderComponent PhysicsRaycast
				if (!raycastedActor) raycastedActor = sceneGraph->MeshRaycast(startPos, endPos);

				//an object was clicked
				if (raycastedActor) {


					if (!keys[SDL_SCANCODE_LCTRL] && !(sceneGraph->debugSelectedAssets.find(raycastedActor->getActorName()) != sceneGraph->debugSelectedAssets.end())) { sceneGraph->debugSelectedAssets.clear(); }

					if (sceneGraph->debugSelectedAssets.find(raycastedActor->getActorName()) != sceneGraph->debugSelectedAssets.end() && keys[SDL_SCANCODE_LCTRL]) { sceneGraph->debugSelectedAssets.erase(raycastedActor->getActorName()); }

					else sceneGraph->debugSelectedAssets.emplace(raycastedActor->getActorName(), raycastedActor);

				}
				//no object was clicked, and left control isn't pressed (making sure the user didn't accidentally misclicked during multi object selection before clearing selection)
				else if (!keys[SDL_SCANCODE_LCTRL])sceneGraph->debugSelectedAssets.clear();



				//startPos.print();
				//endPos.print();
				mouseHeld = false;

			}



			break;
		case SDL_MOUSEBUTTONUP:
			toggleKeyRelease(sdlEvent.button.button);
			std::cout << static_cast<int>(sdlEvent.button.button) << " is Released!" << std::endl;

			break;
		case SDL_MOUSEMOTION:
			if (isHeld(sdlEvent.button.button) || isPressed(sdlEvent.button.button)) {

				// makes it so ImGui handles the mouse motion
				if (GetIO().WantCaptureMouse) {
					toggleKeyPress(sdlEvent.button.button);
					return;
				}

				if (!(sceneGraph->debugSelectedAssets.empty())) {
					//get direction vector of new vector of movement from old mouse pos and new mouse pos

					int deltaX = sdlEvent.motion.x - lastX;
					int deltaY = sdlEvent.motion.y - lastY;
					lastX = sdlEvent.motion.x;
					lastY = sdlEvent.motion.y;

					auto& debugGraph = sceneGraph->debugSelectedAssets;
					Ref<Actor> camera = std::dynamic_pointer_cast<Actor>(sceneGraph->getUsedCamera()->GetUserActor());


					for (const auto& obj : debugGraph) {
						Ref<TransformComponent> transform = obj.second->GetComponent<TransformComponent>();
						Vec3 vectorMove = transform->GetPosition() + Vec3(deltaX, -deltaY, transform->GetPosition().z) * (camera->GetComponent<TransformComponent>()->GetPosition().z - transform->GetPosition().z) / 40 * 0.045;
						transform->SetPos(vectorMove.x, vectorMove.y, transform->GetPosition().z);

					}
				}



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
	InputManager() {};

	// delete copy and move constructers
	InputManager(const InputManager&) = delete;
	InputManager(InputManager&&) = delete;
	InputManager& operator = (const InputManager&) = delete;
	InputManager& operator = (InputManager&&) = delete;

	keyboardInputMap keyboard;
	mouseInputMap mouse;


public:
	

	// Meyers Singleton (from JPs class)
	static InputManager& getInstance() {
		static InputManager instance;
		return instance;
	}

	
	void update(float deltaTime, SceneGraph* sceneGraph) {
		if (GetIO().WantCaptureKeyboard) {
			return;
		}

		//update keyboard object
		keyboard.update(deltaTime);
		mouse.update(deltaTime);
		
		//Check which scene debug vs playing
		if (true) { //temp set to always true until we can define debug vs playable scenes

			//translate selection based on input mapping
			debugInputTranslation({
				{SDL_SCANCODE_W, Vec3(0, 1, 0)},
				{SDL_SCANCODE_S, Vec3(0, -1, 0)},
				{SDL_SCANCODE_A, Vec3(-1, 0, 0)},
				{SDL_SCANCODE_D, Vec3(1, 0, 0)}
			}, sceneGraph);

			//bind keypress to camera and test for swap
			debugInputCamSwap({
				{SDL_SCANCODE_X, sceneGraph->GetActor("cameraActor")->GetComponent<CameraComponent>()},
				{SDL_SCANCODE_C, sceneGraph->GetActor("cameraActor2")->GetComponent<CameraComponent>()}
				}, sceneGraph);

		}
		

	}


	void debugInputCamSwap(std::vector<std::pair<SDL_Scancode, Ref<CameraComponent>>> inputMap, SceneGraph* sceneGraph) {
		
		for (auto& keyPress : inputMap) {
			//key is pressed
			if (keyboard.isPressed(keyPress.first)) {
	
				//set active camera to keypress's binded camera
				if (keyPress.second) {
					sceneGraph->setUsedCamera(keyPress.second);
				}
			}
		}
	}

	/// Allows for a KeyInput to be associated to a translation of a sceneGraph's debug selections
	void debugInputTranslation(std::vector<std::pair<SDL_Scancode, Vec3>> inputMap, SceneGraph* sceneGraph) {
		
		Ref<Actor> camera = (sceneGraph->getUsedCamera()->GetUserActor());
		if (camera) {
			for (auto& keyPress : inputMap) {
				
				if (keyboard.isPressed(keyPress.first)) {

					//Put a slider here for stud based movement
					Vec3 inputVector = keyPress.second * 0.5; //<- slider multiplier here


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
						}
					}
					else {
						camera->GetComponent<TransformComponent>()->SetTransform(
							camera->GetComponent<TransformComponent>()->GetPosition() + worldForward,
							q
						);
						camera->GetComponent<CameraComponent>()->fixCameraToTransform();
					}
				}
			}





		}
	}

	void HandleEvents(const SDL_Event& sdlEvent, SceneGraph* sceneGraph, CollisionSystem* collisionSystem) {

		mouse.HandleEvents(sdlEvent, sceneGraph, collisionSystem);

		
	}


	~InputManager() { }
};
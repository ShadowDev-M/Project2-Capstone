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

enum class InputState {
	Released,
	Pressed,
	Held // For continuous input checks
};

class keyboardInputMap {
private:

	//mapping a keyCode with it's current state
	std::map<int, InputState> keyStates;

public:
	bool isActive(int keyCode) {
		//Active == both pressed and held
		if (keyStates[keyCode] == InputState::Pressed) return true;

		return keyStates[keyCode] == InputState::Held;
	}
	
	bool isPressed(int keyCode) {
		return keyStates[keyCode] == InputState::Pressed;
	}

	bool isReleased(int keyCode) {
		return keyStates[keyCode] == InputState::Released;
	}

	bool isHeld(int keyCode) {
		return keyStates[keyCode] == InputState::Held;
	}

	void toggleKeyPress(int keyCode) {
		keyStates[keyCode] = InputState::Pressed;
	}

	void toggleKeyRelease(int keyCode) {
		keyStates[keyCode] = InputState::Released;
	}

	void toggleKeyHeld(int keyCode) {
		keyStates[keyCode] = InputState::Held;
	}

	void update(float deltaTime) {
		
		
		
		
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


public:
	

	// Meyers Singleton (from JPs class)
	static InputManager& getInstance() {
		static InputManager instance;
		return instance;
	}

	
	void update(float deltaTime, SceneGraph* sceneGraph) {

		//update keyboard object
		keyboard.update(deltaTime);

		
		//Check which scene debug vs playing
		if (true) { //temp set to always true until we can define debug vs playable scenes

			//translate selection based on input mapping
			debugInputTranslation({
				{SDL_SCANCODE_W, Vec3(0, 1, 0)},
				{SDL_SCANCODE_S, Vec3(0, -1, 0)},
				{SDL_SCANCODE_A, Vec3(-1, 0, 0)},
				{SDL_SCANCODE_D, Vec3(1, 0, 0)}
			}, sceneGraph);
		}
		

	}

	/// Allows for a KeyInput to be associated to a translation of a sceneGraph's debug selections
	void debugInputTranslation(std::vector<std::pair<SDL_Scancode, Vec3>> inputMap, SceneGraph* sceneGraph) {
		
		Ref<CameraActor> camera = std::dynamic_pointer_cast<CameraActor>(sceneGraph->GetActor("camera"));
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
						camera->fixCameraToTransform();
					}
				}
			}





		}
	}

	~InputManager() { }
};
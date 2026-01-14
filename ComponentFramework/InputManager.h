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
#include "imgui.h"
#include "CollisionSystem.h"
#include <functional>
#include <tuple>
#include <utility>
#include <thread>
#include <chrono>
#include "ImGuizmo.h"
#include <VMath.h>
#include <Matrix.h>
#include <SDL.h>
#include <MMath.h>

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

	///Calls the function associated with the binding
	void call() override;
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
	std::vector<PoolBindObject*> getBindings(KeyBinding keybinding);

	///call all poolObjects that share the parametre's keybinding
	void call(KeyBinding keybinding);

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

	void HandleEvents(const SDL_Event& sdlEvent, SceneGraph* sceneGraph);

	void update(const float deltaTime) override;
};

class keyboardInputMap : public InputMap {
private:

	
public:

	void update(const float deltaTime) override;

};

class gamepadInputMap {
private:
	SDL_GameController* controller;
	
	// finds a controller connected (controller has to be connected before running the engine)
	SDL_GameController* findController();

	// joystick states
	float leftStickX = 0.0f;
	float leftStickY = 0.0f;
	// button states
	bool leftShoulderPressed = false;
	bool rightShoulderPressed = false;

	// deadzone for joystick
	const float deadzone = 0.2f;

	float applyDeadzone(float value);

public: 
	gamepadInputMap() {
		controller = findController();
		if (!controller) {
			std::cout << "No controller found." << std::endl;
		}
	}
	
	~gamepadInputMap() {
		if (controller) {
			SDL_GameControllerClose(controller);
			controller = nullptr;
		}
	}
	
	// checks if controller is currently connected
	bool isConnected() const {
		return controller != nullptr && SDL_GameControllerGetAttached(controller);
	}

	void HandleEvents(const SDL_Event& event);
	
	// for joystick movement
	void HandleAxisMotion(Uint8 axis, Sint16 value);

	void Update(float deltaTime, SceneGraph* sceneGraph_);
};

class InputManager
{
private:
	InputManager();
	// delete copy and move constructers
	InputManager(const InputManager&) = delete;
	InputManager(InputManager&&) = delete;
	InputManager& operator = (const InputManager&) = delete;
	InputManager& operator = (InputManager&&) = delete;

	keyboardInputMap keyboard;
	mouseInputMap mouse;
	gamepadInputMap gamepad;


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

	
	void update(float deltaTime, SceneGraph* sceneGraph);

	// getter and setter for stud multipler (used in slider)
	float GetStudMultiplier() { return studMultiplier; }
	void SetStudMultiplier(float studMulti_) { studMultiplier = studMulti_; }

	bool startGame(std::pair<KeyBinding, std::tuple<bool>> input, SceneGraph* sceneGraph);

	void debugInputCamSwap(std::vector<std::pair<SDL_Scancode, Ref<CameraComponent>>> inputMap, SceneGraph* sceneGraph);

	/// Allows for a KeyInput to be associated to a translation of a sceneGraph's debug selections
	bool debugTapInputTranslation(std::pair<KeyBinding, std::tuple<Vec3>> inputMap, SceneGraph* sceneGraph);

	bool debugCamInputTranslation(std::pair<KeyBinding, std::tuple<Vec3>> inputMap, SceneGraph* sceneGraph);

	void HandleEvents(const SDL_Event& sdlEvent, SceneGraph* sceneGraph) {
		sceneGraph->checkValidCamera();

		mouse.HandleEvents(sdlEvent, sceneGraph);

		gamepad.HandleEvents(sdlEvent);
	}


	~InputManager() { }
};


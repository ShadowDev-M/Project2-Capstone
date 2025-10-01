#include <SDL.h>
#include "SceneManager.h"
#include "Timer.h"
#include "Window.h"
#include "Scene3GUI.h"
#include "EditorManager.h"
#include "SceneGraph.h"
#include "Scene4Lights.h"

SceneManager::SceneManager() :
	currentScene{ nullptr }, window{ nullptr }, timer{ nullptr },
	fps(60), isRunning{ false }, fullScreen{ false } {
	Debug::Info("Starting the SceneManager", __FILE__, __LINE__);
}

SceneManager::~SceneManager() {
	Debug::Info("Deleting the SceneManager", __FILE__, __LINE__);

	if (EditorManager::getInstance().IsInitialized()) {
		EditorManager::getInstance().Shutdown();
	}

	if (currentScene) {
		currentScene->OnDestroy();
		delete currentScene;
		currentScene = nullptr;
	}

	if (timer) {
		delete timer;
		timer = nullptr;
	}

	if (window) {
		delete window;
		window = nullptr;
	}

}

bool SceneManager::Initialize(std::string name_, int width_, int height_) {

	window = new Window();
	if (!window->OnCreate(name_, width_, height_)) {
		Debug::FatalError("Failed to initialize Window object", __FILE__, __LINE__);
		return false;
	}

	timer = new Timer();
	if (timer == nullptr) {
		Debug::FatalError("Failed to initialize Timer object", __FILE__, __LINE__);
		return false;
	}

	/********************************   Default first scene   ***********************/
	BuildNewScene(SCENE_NUMBER::SCENE3GUI);
	/********************************************************************************/
	
	EditorManager& editor = EditorManager::getInstance();
	if (!editor.Initialize(window->getWindow(), window->getContext(), &SceneGraph::getInstance())) {
		Debug::FatalError("Failed to initialize EditorManager", __FILE__, __LINE__);
		return false;
	}

	return true;
}

/// This is the whole game
void SceneManager::Run() {
	timer->Start();
	isRunning = true;

	EditorManager& editor = EditorManager::getInstance();

	while (isRunning) {
		HandleEvents();
		timer->UpdateFrameTicks();
		currentScene->Update(timer->GetDeltaTime());
		currentScene->Render();

		editor.RenderEditorUI();

		SDL_GL_SwapWindow(window->getWindow());
		SDL_Delay(timer->GetSleepTime(fps));
	}
}

void SceneManager::HandleEvents() {
	EditorManager& editor = EditorManager::getInstance();

	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent)) { /// Loop over all events in the SDL queue

		editor.HandleEvents(sdlEvent);

		if (sdlEvent.type == SDL_EventType::SDL_QUIT) {
			isRunning = false;
			return;
		}
		else if (sdlEvent.type == SDL_KEYDOWN) {
			switch (sdlEvent.key.keysym.scancode) {
				[[fallthrough]]; /// C17 Prevents switch/case fallthrough warnings
			case SDL_SCANCODE_ESCAPE:
			case SDL_SCANCODE_Q:
				isRunning = false;
				return;

			case SDL_SCANCODE_F1:
				BuildNewScene(SCENE_NUMBER::SCENE3GUI);
				break;

			case SDL_SCANCODE_F2:
				BuildNewScene(SCENE_NUMBER::SCENE4LIGHTS);
				break;

			default:
				break;
			}
		}
		if (currentScene == nullptr) { /// Just to be careful
			Debug::FatalError("No currentScene", __FILE__, __LINE__);
			isRunning = false;
			return;
		}
		currentScene->HandleEvents(sdlEvent);
	}
}

bool SceneManager::BuildNewScene(SCENE_NUMBER scene) {
	bool status;

	if (currentScene != nullptr) {
		currentScene->OnDestroy();
		delete currentScene;
		currentScene = nullptr;
	}

	switch (scene) {
	case SCENE_NUMBER::SCENE3GUI:
		currentScene = new Scene3GUI();
		status = currentScene->OnCreate();
		break;

	case SCENE_NUMBER::SCENE4LIGHTS:
		currentScene = new Scene4Lights();
		status = currentScene->OnCreate();
		break;

	default:
		Debug::Error("Incorrect scene number assigned in the manager", __FILE__, __LINE__);
		currentScene = nullptr;
		return false;
	}
	return true;
}



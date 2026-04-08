#include "pch.h"
#include "SceneManager.h"
#include "Timer.h"
#include "Window.h"
#include "Scene3GUI.h"
#include "EditorManager.h"
#include "SceneGraph.h"
#include "ScreenManager.h"
#include "FBOManager.h"
#include "AnimationSystem.h"
#include "Renderer.h"
#include "InputManager.h"
#include "PhysicsSystem.h"

SceneManager::SceneManager() :
	currentScene{ nullptr }, window{ nullptr }, timer{ nullptr },
	isRunning{ false }, fullScreen{ false } {
	Debug::Info("Starting the SceneManager", __FILE__, __LINE__);
}

SceneManager::~SceneManager() {
	Debug::Info("Deleting the SceneManager", __FILE__, __LINE__);

	if (EditorManager::getInstance().IsInitialized()) {
		EditorManager::getInstance().Shutdown();
	}

	Renderer::getInstance().OnDestroy();
	FBOManager::getInstance().OnDestroy();

	if (currentScene) {
		currentScene->OnDestroy();
		delete currentScene;
		currentScene = nullptr;
	}

	if (timer) {
		delete timer;
		timer = nullptr;
	}

	AnimationSystem::getInstance().StopMeshLoadingWorker();
	AssetManager::getInstance().RemoveAllAssets();
	SceneGraph::getInstance().OnDestroy();

	if (window) {
		delete window;
		window = nullptr;
	}
}

bool SceneManager::Initialize(std::string name_, int width_, int height_) {
	SearchPath::getInstance().Initialize(fs::current_path() / "Assets");

	window = new Window();
	if (!window->OnCreate(name_, width_, height_)) {
		Debug::FatalError("Failed to initialize Window object", __FILE__, __LINE__);
		return false;
	}

	// filling out config file & initializing screenmanager
	SettingsConfig cfg;
	cfg.windowTitle = name_;
	cfg.renderWidth = width_;
	cfg.renderHeight = height_;
	cfg.displayWidth = width_;
	cfg.displayHeight = height_;
	ScreenManager::getInstance().Initialize(window->getWindow(), cfg);

	// creating FBOs
#ifdef ENGINE_EDITOR
	FBOManager::getInstance().CreateFBO(FBO::Scene, width_, height_);
	FBOManager::getInstance().CreateFBO(FBO::Game, width_, height_);
	FBOManager::getInstance().CreateFBO(FBO::ColorPicker, width_, height_);
	FBOManager::getInstance().CreateShadowFBO(FBO::ShadowMap, 1024, 1024);
	FBOManager::getInstance().CreateShadowFBO(FBO::ShadowCubeMap, 1024, 1024, true);
	FBOManager::getInstance().CreateShadowFBO(FBO::ShadowCubeMap1, 1024, 1024, true);
	FBOManager::getInstance().CreateShadowFBO(FBO::ShadowCubeMap2, 1024, 1024, true);
	FBOManager::getInstance().CreateShadowFBO(FBO::ShadowCubeMap3, 1024, 1024, true);
#endif

	if (!Renderer::getInstance().OnCreate()) {
		return false;
	}

	timer = new Timer();
	if (timer == nullptr) {
		Debug::FatalError("Failed to initialize Timer object", __FILE__, __LINE__);
		return false;
	}

	AnimationSystem::getInstance().StartMeshLoadingWorker();

	AssetManager::getInstance().Initialize();

	/********************************   Default first scene   ***********************/
	BuildNewScene(SCENE_NUMBER::SCENE3GUI);
	/********************************************************************************/
	
#ifdef ENGINE_EDITOR
	EditorManager& editor = EditorManager::getInstance();
	if (!editor.Initialize(window->getWindow(), window->getContext(), &SceneGraph::getInstance())) {
		Debug::FatalError("Failed to initialize EditorManager", __FILE__, __LINE__);
		return false;
	}
#endif

	return true;
}

/// This is the whole game
void SceneManager::Run() {
	timer->Start();
	isRunning = true;

	EditorManager& editor = EditorManager::getInstance();
	const SettingsConfig& cfg = ScreenManager::getInstance().getConfig();

	while (isRunning) {
		timer->UpdateFrameTicks();

		AnimationSystem::getInstance().ProcessMainThreadTasks();
		HandleEvents();
		Update(timer->GetDeltaTime());
		Render();
		editor.RenderEditorUI();
		SDL_GL_SwapWindow(window->getWindow());
		
		timer->LimitFrameRate(cfg.targetFPS, cfg.vsync);
	}
}

void SceneManager::Update(float deltaTime) {
	AnimationSystem::getInstance().Update(deltaTime);
	InputManager::getInstance().update(deltaTime);
	SceneGraph::getInstance().Update(deltaTime);
	
	// remove if causing bugs
	//bool windowFocused = (SDL_GetWindowFlags(window->getWindow()) & SDL_WINDOW_INPUT_FOCUS) != 0;

#ifdef ENGINE_EDITOR
	if (EditorManager::getInstance().isPlayMode()) {
		PhysicsSystem::getInstance().Update(deltaTime);
		CollisionSystem::getInstance().Update(deltaTime);
	}
#else
	PhysicsSystem::getInstance().Update(deltaTime);
	CollisionSystem::getInstance().Update(deltaTime);
#endif

	// if (currentScene) currentScene->Update(deltaTime);
}

void SceneManager::Render() {
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	Renderer::getInstance().ShadowPass();
	Renderer::getInstance().RenderSceneView();
	Renderer::getInstance().RenderGameView();
}

void SceneManager::ProcessPendingLoad()
{
	if (!SceneLoader::getInstance().HasPending()) return;

	SceneLoader::Request request = SceneLoader::getInstance().ConsumePending();
	using Type = SceneLoader::RequestType;

	std::string targetScene;
	if (request.type == Type::ByName) {
		targetScene = request.name;
	}
	else if (request.type == Type::ById) {
		const auto* s = ProjectSettingsManager::getInstance().Get().GetSceneById(request.id);
		if (!s) return;
		targetScene = s->name;
	}
	else if (request.type == Type::Next) {
		int nextId = SceneLoader::GetActiveSceneId() + 1;
		const auto* s = ProjectSettingsManager::getInstance().Get().GetSceneById(nextId);
		if (!s) return;
		targetScene = s->name;
	}

	if (targetScene.empty()) return;

	LoadSceneFile(targetScene);

#ifdef ENGINE_EDITOR
	if (EditorManager::getInstance().isPlayMode()) {
		SceneGraph::getInstance().Start();
		InputManager::getInstance().setGameInputActive(true);
	}
#else
	SceneGraph::getInstance().Start();
#endif
}

void SceneManager::LoadSceneFile(const std::string& sceneName)
{
	// shutdown
	AnimationSystem::getInstance().StopMeshLoadingWorker();
	LightingSystem::getInstance().ClearActors();
	PhysicsSystem::getInstance().ClearActors();
	CollisionSystem::getInstance().ClearActors();
	SceneGraph::getInstance().OnDestroy();

	// load new scene
	AnimationSystem::getInstance().StartMeshLoadingWorker();
	SceneGraph::getInstance().OnCreate();
	SceneGraph::getInstance().sceneFileName = sceneName;
	XMLObjectFile::addActorsFromFile(&SceneGraph::getInstance(), sceneName.c_str());
	ScreenManager::getInstance().setWindowTitle(sceneName);

	ProjectSettings& cfg = ProjectSettingsManager::getInstance().Get();
	int sceneId = cfg.GetSceneIdByName(sceneName);
	if (sceneId == -1) {
		fs::path sceneDir = SearchPath::getInstance().EnsureSubfolder("Scenes");
		fs::path scenePath = sceneDir / (sceneName + ".scene");
		std::string relPath = SearchPath::getInstance().MakeRelative(scenePath).string();

		SceneListData entry;
		entry.name = sceneName;
		entry.path = relPath;
		cfg.sceneList.push_back(entry);
		cfg.RebuildIds();
		ProjectSettingsManager::getInstance().SaveDefault();
		sceneId = cfg.GetSceneIdByName(sceneName);
	}

	SceneLoader::SetActiveScene(sceneName, sceneId);
	Debug::Info("Loaded scene: " + sceneName + " (id=" + std::to_string(sceneId) + ")", __FILE__, __LINE__);
}

void SceneManager::HandleEvents() {
	EditorManager& editor = EditorManager::getInstance();

	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent)) { /// Loop over all events in the SDL queue

		editor.HandleEvents(sdlEvent);

		if (sdlEvent.type == SDL_WINDOWEVENT && sdlEvent.window.event == SDL_WINDOWEVENT_RESIZED) {
			ScreenManager::getInstance().HandleResize(sdlEvent.window.data1, sdlEvent.window.data2, Source::WindowDrag);
		}

		if (sdlEvent.type == SDL_EventType::SDL_QUIT) {
			isRunning = false;
			return;
		}
		else if (sdlEvent.type == SDL_KEYDOWN) {
			switch (sdlEvent.key.keysym.scancode) {
				[[fallthrough]]; /// C17 Prevents switch/case fallthrough warnings
			

			case SDL_SCANCODE_F1:
				BuildNewScene(SCENE_NUMBER::SCENE3GUI);
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

	default:
		Debug::Error("Incorrect scene number assigned in the manager", __FILE__, __LINE__);
		currentScene = nullptr;
		return false;
	}
	return true;
}



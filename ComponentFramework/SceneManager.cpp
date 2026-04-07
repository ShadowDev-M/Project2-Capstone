#include "pch.h"
#include "SceneManager.h"
#include "Timer.h"
#include "Window.h"
#include "EditorManager.h"
#include "SceneGraph.h"
#include "ScreenManager.h"
#include "FBOManager.h"
#include "AnimationSystem.h"
#include "Renderer.h"
#include "InputManager.h"
#include "PhysicsSystem.h"
#include "ColliderDebug.h"
#include "ProjectSettingsManager.h"
#include "SceneLoader.h"
#include "LightingSystem.h"

SceneManager::SceneManager() :
	window{ nullptr }, timer{ nullptr }, isRunning{ false } {
	Debug::Info("Starting the SceneManager", __FILE__, __LINE__);
}

SceneManager::~SceneManager() {
	Debug::Info("Deleting the SceneManager", __FILE__, __LINE__);

	LightingSystem::getInstance().ClearActors();
	PhysicsSystem::getInstance().ClearActors();
	CollisionSystem::getInstance().ClearActors();
	SceneGraph::getInstance().OnDestroy();
	AnimationSystem::getInstance().StopMeshLoadingWorker();
	// AudioManager::getInstance().Shutdown();

#ifdef ENGINE_EDITOR
	ColliderDebug::getInstance().OnDestroy();

	if (EditorManager::getInstance().IsInitialized()) {
		EditorManager::getInstance().Shutdown();
	}
#endif

	Renderer::getInstance().OnDestroy();
	FBOManager::getInstance().OnDestroy();
	AssetManager::getInstance().RemoveAllAssets();

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
	SearchPath::getInstance().Initialize(fs::current_path() / "Assets");
	SearchPath::getInstance().InitializeEngineAssets(fs::current_path() / "EngineAssets");
	
	ProjectSettings& cfg = ProjectSettingsManager::getInstance().Get();	
	cfg.windowTitle = name_;
	cfg.renderWidth = width_;
	cfg.renderHeight = height_;
	cfg.displayWidth = width_;
	cfg.displayHeight = height_;

	ProjectSettingsManager::getInstance().LoadDefault();

	window = new Window();
	if (!window->OnCreate(cfg.windowTitle, cfg.displayWidth, cfg.displayHeight)) {
		Debug::FatalError("Failed to initialize Window object", __FILE__, __LINE__);
		return false;
	}
	ScreenManager::getInstance().Initialize(window->getWindow());

	// creating FBOs
#ifdef ENGINE_EDITOR
	FBOManager::getInstance().CreateFBO(FBO::Scene, cfg.renderWidth, cfg.renderHeight);
	FBOManager::getInstance().CreateFBO(FBO::Game, cfg.renderWidth, cfg.renderHeight);
	FBOManager::getInstance().CreateFBO(FBO::ColorPicker, cfg.renderWidth, cfg.renderHeight);
	FBOManager::getInstance().CreateShadowFBO(FBO::ShadowMap, 1024, 1024);
	FBOManager::getInstance().CreateShadowFBO(FBO::ShadowCubeMap, 1024, 1024, true);
	FBOManager::getInstance().CreateShadowFBO(FBO::ShadowCubeMap1, 1024, 1024, true);
	FBOManager::getInstance().CreateShadowFBO(FBO::ShadowCubeMap2, 1024, 1024, true);
	FBOManager::getInstance().CreateShadowFBO(FBO::ShadowCubeMap3, 1024, 1024, true);
#endif

	if (!Renderer::getInstance().OnCreate()) { return false; }

	timer = new Timer();
	if (timer == nullptr) {
		Debug::FatalError("Failed to initialize Timer object", __FILE__, __LINE__);
		return false;
	}

	AnimationSystem::getInstance().StartMeshLoadingWorker();
	AssetManager::getInstance().Initialize();
	AssetManager::getInstance().LoadEngineAssets();
	ColliderDebug::getInstance().OnCreate();

	{
		const SceneListData* startup = cfg.GetStartupScene();
		if (startup) {
			LoadSceneFile(startup->name);
		}
		else {
			fs::path dir = SearchPath::getInstance().EnsureSubfolder("Scenes");
			fs::path existingScene = dir / "SampleScene.scene";
			std::string stem;

			if (fs::exists(existingScene)) {
				stem = "SampleScene";
			}
			else {
				stem = AssetManager::getInstance().GenerateUniqueFileName(dir, "SampleScene", ".scene");
				fs::path out = dir / (stem + ".scene");

				XMLDocument doc;
				auto* root = doc.NewElement("Scene");
				root->InsertEndChild(doc.NewElement("Actors"));
				doc.InsertFirstChild(root);
				doc.SaveFile(out.string().c_str());
			}

			LoadSceneFile(stem);
		}
	}
	
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
	const ProjectSettings& cfg = ScreenManager::getInstance().getConfig();

	while (isRunning) {
		timer->UpdateFrameTicks();
		AnimationSystem::getInstance().ProcessMainThreadTasks();
		HandleEvents();
		Update(timer->GetDeltaTime());
		Render();

#ifdef ENGINE_EDITOR
		editor.RenderEditorUI();
#endif

		SDL_GL_SwapWindow(window->getWindow());
		timer->LimitFrameRate(cfg.targetFPS, cfg.vsync);
	}
}

void SceneManager::Update(float deltaTime) {
	// is there a better way then just having it be constantly called in update
	ProcessPendingLoad();

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
	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent)) { /// Loop over all events in the SDL queue

#ifdef ENGINE_EDITOR
		EditorManager::getInstance().HandleEvents(sdlEvent);
#endif

		InputManager::getInstance().HandleEvents(sdlEvent);

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

			default:
				break;
			}
		}
	}
}
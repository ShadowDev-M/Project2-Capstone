#include "pch.h"
#include "Scene3GUI.h"


#include "XMLManager.h"
#include "InputManager.h"
#include "CameraComponent.h"
#include "MemorySize.h"
#include "AnimatorComponent.h"

#include "PhysicsSystem.h"
#include "CollisionSystem.h"
#include "ColliderDebug.h"
#include "ScreenManager.h"
#include "AnimationSystem.h"

Scene3GUI::Scene3GUI() {
	Debug::Info("Created Scene3GUI: ", __FILE__, __LINE__);
}

Scene3GUI::~Scene3GUI() {
	Debug::Info("Deleted Scene3GUI: ", __FILE__, __LINE__);
}

bool Scene3GUI::OnCreate() {
	Debug::Info("Loading assets Scene3GUI: ", __FILE__, __LINE__);

	AssetManager::getInstance().OnCreate();	
	SceneGraph::getInstance().OnCreate();

	std::vector<std::string> sceneTags = XMLObjectFile::readSceneTags(SceneGraph::getInstance().sceneFileName);
	for (const auto& tag : sceneTags) {
		SceneGraph::getInstance().addTag(tag);
	}

	XMLObjectFile::addActorsFromFile(&SceneGraph::getInstance(), "LevelThree");
	ScreenManager::getInstance().setWindowTitle(SceneGraph::getInstance().sceneFileName);

	ColliderDebug::getInstance().OnCreate();

	//AudioManager::getInstance().Initialize();
	//marioSFX = AudioManager::getInstance().Play3D("audio/mario.wav", SceneGraph::getInstance().GetActor("Mario")->GetComponent<TransformComponent>()->GetPosition(), true);
	return true;
}


void Scene3GUI::OnDestroy() {
	Debug::Info("Deleting assets Scene3GUI: ", __FILE__, __LINE__);

	// save all the assets in the assetmanager to the xml file then remove them all locally
	AssetManager::getInstance().SaveAssetDatabaseXML();
	AssetManager::getInstance().RemoveAllAssets();

	ColliderDebug::getInstance().OnDestroy();

	SceneGraph::getInstance().RemoveAllActors();

	AudioManager::getInstance().Shutdown();
	MemoryStale();
}

void Scene3GUI::HandleEvents(const SDL_Event& sdlEvent) {
	InputManager::getInstance().HandleEvents(sdlEvent, &SceneGraph::getInstance());

	//if (InputManager::getInstance().getKeyboardMap()->isPressed(SDL_SCANCODE_E)) {
	//	std::cout << "E";
	//	ScreenManager::getInstance().HandleResize(720, 480, Source::Script);
	//}
}


void Scene3GUI::Update(const float deltaTime) {
	
}

void Scene3GUI::Render() const {
}
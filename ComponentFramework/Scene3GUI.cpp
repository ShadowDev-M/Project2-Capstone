#include "pch.h"
#include "Scene3GUI.h"


#include "XMLManager.h"
#include "InputManager.h"
#include "CameraComponent.h"
#include "MemorySize.h"
#include "AnimatorComponent.h"
#include "PhysicsSystem.h"



Scene3GUI::Scene3GUI() {
	Debug::Info("Created Scene3GUI: ", __FILE__, __LINE__);
}

Scene3GUI::~Scene3GUI() {
	Debug::Info("Deleted Scene3GUI: ", __FILE__, __LINE__);
}

bool Scene3GUI::OnCreate() {
	Debug::Info("Loading assets Scene3GUI: ", __FILE__, __LINE__);

	AssetManager::getInstance().OnCreate();	
	
	SceneGraph::getInstance().checkValidCamera();
	
	SceneGraph::getInstance().OnCreate();
	
	//AssetManager::getInstance().ListAllAssets();
	//SceneGraph::getInstance().ListAllActors();

	XMLObjectFile::addActorsFromFile(&SceneGraph::getInstance(), "LevelThree");

	//AudioManager::getInstance().Initialize();
	//marioSFX = AudioManager::getInstance().Play3D("audio/mario.wav", SceneGraph::getInstance().GetActor("Mario")->GetComponent<TransformComponent>()->GetPosition(), true);
	//SceneGraph::getInstance().GetActor("Mario")->AddComponent<AnimatorComponent>((SceneGraph::getInstance().GetActor("Mario").get()));
	return true;
}


void Scene3GUI::OnDestroy() {
	Debug::Info("Disabling Mesh-Loading Thread", __FILE__, __LINE__);

	SceneGraph::getInstance().stopMeshLoadingWorker();

	Debug::Info("Deleting assets Scene3GUI: ", __FILE__, __LINE__);

	


	// save all the assets in the assetmanager to the xml file then remove them all locally
	AssetManager::getInstance().SaveAssetDatabaseXML();
	AssetManager::getInstance().RemoveAllAssets();

	SceneGraph::getInstance().RemoveAllActors();

	AudioManager::getInstance().Shutdown();
}

void Scene3GUI::HandleEvents(const SDL_Event& sdlEvent) {
	InputManager::getInstance().HandleEvents(sdlEvent, &SceneGraph::getInstance());
}


void Scene3GUI::Update(const float deltaTime) {
	
	InputManager::getInstance().update(deltaTime, &SceneGraph::getInstance());
	
	SceneGraph::getInstance().Update(deltaTime);

	if (EditorManager::getInstance().isPlayMode()) {
		PhysicsSystem::getInstance().Update(deltaTime);
		
		//SceneGraph::getInstance().GetActor("Cube")->GetComponent<PhysicsComponent>()->ApplyForce(Vec3(0.0f, -9.8f, 0.0f));
		//std::cout << SceneGraph::getInstance().GetActor("Cube")->GetComponent<PhysicsComponent>()->getMass();
	}

	Ref<Actor> cameraActor = SceneGraph::getInstance().getUsedCamera()->GetUserActor();
	if (cameraActor && cameraActor->GetComponent<TransformComponent>()) {
		Vec3 cameraPos = cameraActor->GetComponent<TransformComponent>()->GetPosition();
		Vec3 lookDir = cameraActor->GetComponent<TransformComponent>()->GetForward();
		AudioManager::getInstance().SetListenerPos(cameraPos, lookDir);
	}
	
//	AudioManager::getInstance().SetSoundPos(marioSFX, SceneGraph::getInstance().GetActor("Mario")->GetComponent<TransformComponent>()->GetPosition());
}

void Scene3GUI::Render() const {
	/// Set the background color then clear the screen
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Rendering	
	glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetProgram());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetUniformID("projectionMatrix"), 1, GL_FALSE, SceneGraph::getInstance().getUsedCamera()->GetProjectionMatrix());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetUniformID("viewMatrix"), 1, GL_FALSE, SceneGraph::getInstance().getUsedCamera()->GetViewMatrix());

	glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_AnimOutline")->GetProgram());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_AnimOutline")->GetUniformID("projectionMatrix"), 1, GL_FALSE, SceneGraph::getInstance().getUsedCamera()->GetProjectionMatrix());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_AnimOutline")->GetUniformID("viewMatrix"), 1, GL_FALSE, SceneGraph::getInstance().getUsedCamera()->GetViewMatrix());


	glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetProgram());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetUniformID("projectionMatrix"), 1, GL_FALSE, SceneGraph::getInstance().getUsedCamera()->GetProjectionMatrix());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetUniformID("viewMatrix"), 1, GL_FALSE, SceneGraph::getInstance().getUsedCamera()->GetViewMatrix());


	glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Multi")->GetProgram());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Multi")->GetUniformID("projectionMatrix"), 1, GL_FALSE, SceneGraph::getInstance().getUsedCamera()->GetProjectionMatrix());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Multi")->GetUniformID("viewMatrix"), 1, GL_FALSE, SceneGraph::getInstance().getUsedCamera()->GetViewMatrix());
	
	SceneGraph::getInstance().Render();

	glUseProgram(0);
}
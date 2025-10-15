#include <glew.h>
#include <iostream>
#include <SDL.h>
#include "Scene3GUI.h"
#include <MMath.h>
#include "Debug.h"
#include "ExampleXML.h"
#include "ScriptComponent.h"

#include "InputManager.h"
#include "CameraComponent.h"
#include <filesystem>

Scene3GUI::Scene3GUI() : drawInWireMode{ false } {
	Debug::Info("Created Scene3GUI: ", __FILE__, __LINE__);
}

Scene3GUI::~Scene3GUI() {
	Debug::Info("Deleted Scene3GUI: ", __FILE__, __LINE__);
}

bool Scene3GUI::OnCreate() {
	Debug::Info("Loading assets Scene3GUI: ", __FILE__, __LINE__);

	AssetManager::getInstance().OnCreate();	
	AssetManager::getInstance().ListAllAssets();
	
	SceneGraph::getInstance().checkValidCamera();
	
	// Light Pos
	lightPos = Vec3(1.0f, 2.0f, -10.0f);

	SceneGraph::getInstance().OnCreate();
	SceneGraph::getInstance().ListAllActors();

	ScriptManager::Push_Script("test");

	ScriptManager::OpenFileForUser("test");


	//	camera->fixCameraToTransform();
	XMLObjectFile::addActorsFromFile(&SceneGraph::getInstance(), "LevelThree");


	return true;
}


void Scene3GUI::OnDestroy() {
	Debug::Info("Deleting assets Scene3GUI: ", __FILE__, __LINE__);

	// save all the assets in the assetmanager to the xml file then remove them all locally
	AssetManager::getInstance().SaveAssetDatabaseXML();
	AssetManager::getInstance().RemoveAllAssets();

	SceneGraph::getInstance().RemoveAllActors();

	//camera->OnDestroy();

}

void Scene3GUI::HandleEvents(const SDL_Event& sdlEvent) {

	//sceneGraph.checkValidCamera();

	InputManager::getInstance().HandleEvents(sdlEvent, &SceneGraph::getInstance(), &collisionSystem);

}


void Scene3GUI::Update(const float deltaTime) {

	for (auto& scriptObj : ScriptManager::scriptsRuntimeVector) {
		scriptObj->Start();
	}


	InputManager::getInstance().update(deltaTime, &SceneGraph::getInstance());
	
	collisionSystem.Update(deltaTime);

	SceneGraph::getInstance().Update(deltaTime);
}

void Scene3GUI::Render() const {
	/// Set the background color then clear the screen
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	if (drawInWireMode) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// Rendering	
	glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetProgram());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetUniformID("projectionMatrix"), 1, GL_FALSE, SceneGraph::getInstance().getUsedCamera()->GetProjectionMatrix());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetUniformID("viewMatrix"), 1, GL_FALSE, SceneGraph::getInstance().getUsedCamera()->GetViewMatrix());

	glUniform3fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetUniformID("lightPos"), 1, lightPos);


	// S_Multi works now, however light components don't get saved to XML yet so whenever you start up the engine it'll just be dark
	glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong")->GetProgram());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong")->GetUniformID("projectionMatrix"), 1, GL_FALSE, SceneGraph::getInstance().getUsedCamera()->GetProjectionMatrix());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong")->GetUniformID("viewMatrix"), 1, GL_FALSE, SceneGraph::getInstance().getUsedCamera()->GetViewMatrix());

	glUniform3fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong")->GetUniformID("lightPos"), 1, lightPos);
	
	SceneGraph::getInstance().Render();

	glUseProgram(0);
}
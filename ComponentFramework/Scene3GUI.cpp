#include <glew.h>
#include <iostream>
#include <SDL.h>
#include "Scene3GUI.h"
#include <MMath.h>
#include "Debug.h"
#include "ExampleXML.h"
#include "InputManager.h"

using namespace ImGui;

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

	camera = std::make_shared<CameraActor>(nullptr, 45.0f, (16.0f / 9.0f), 0.5f, 100.0f);
	
	camera->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, 40.0f), QMath::inverse( Quaternion()));
	
	camera->OnCreate();
	
	sceneGraph.AddActor(camera);

	//example.readDoc();

	camera->fixCameraToTransform();
	
	// Light Pos
	lightPos = Vec3(1.0f, 2.0f, -10.0f);

	// Board setup

	//sceneGraph.LoadActor("Board");




	//sceneGraph.LoadActor("Mario", sceneGraph.GetActor("Board"));

	
	

	//sceneGraph.LoadActor("Sphere", sceneGraph.GetActor("Board"));




	sceneGraph.OnCreate();

	sceneGraph.ListAllActors();

	//sceneGraph.RemoveActor("Sphere");
	camera->fixCameraToTransform();
	XMLObjectFile::addActorsFromFile(&sceneGraph, "LevelThree");

	// pass along the scene graph to the windows
	hierarchyWindow = std::make_unique<HierarchyWindow>(&sceneGraph);
	inspectorWindow = std::make_unique<InspectorWindow>(&sceneGraph);
	assetManagerWindow = std::make_unique<AssetManagerWindow>(&sceneGraph);


	return true;
}


void Scene3GUI::OnDestroy() {
	Debug::Info("Deleting assets Scene3GUI: ", __FILE__, __LINE__);

	// save all the assets in the assetmanager to the xml file then remove them all locally
	AssetManager::getInstance().SaveAssetDatabaseXML();
	AssetManager::getInstance().RemoveAllAssets();

	sceneGraph.RemoveAllActors();

	camera->OnDestroy();

	if (hierarchyWindow) {
		hierarchyWindow->ClearFilter();
	}
	if (assetManagerWindow) {
		assetManagerWindow->ClearFilter();
	}

}

void Scene3GUI::HandleEvents(const SDL_Event& sdlEvent) {

	InputManager::getInstance().HandleEvents(sdlEvent, &sceneGraph, &collisionSystem);

	
}


void Scene3GUI::Update(const float deltaTime) {

	InputManager::getInstance().update(deltaTime, &sceneGraph);
	
	collisionSystem.Update(deltaTime);

	sceneGraph.Update(deltaTime);
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

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	if (show_demo_window) {
		ImGui::ShowDemoWindow(&show_demo_window);
	}
			
	if (show_hierarchy_window) {
		hierarchyWindow->ShowHierarchyWindow(&show_hierarchy_window);
	}

	if (show_inspector_window) {
		inspectorWindow->ShowInspectorWindow(&show_inspector_window);
	}

	if (show_assetmanager_window) {
		assetManagerWindow->ShowAssetManagerWindow(&show_assetmanager_window);
	}

	if (BeginMainMenuBar()) {
		if (BeginMenu("File")) {
			if (MenuItem("Save", "Ctrl+S")) {
				sceneGraph.SaveFile("LevelThree");
			}
			EndMenu();
		}

		if (BeginMenu("Windows")) {
			MenuItem("Demo", nullptr, &show_demo_window);
			MenuItem("Hierarchy", nullptr, &show_hierarchy_window);
			MenuItem("Inspector", nullptr, &show_inspector_window);
			MenuItem("Asset Manager", nullptr, &show_assetmanager_window);

			EndMenu();
		}
		EndMainMenuBar();
	}
	

	// Rendering
	ImGui::Render();
	glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
	glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	

	glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetProgram());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetUniformID("projectionMatrix"), 1, GL_FALSE, camera->GetProjectionMatrix());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetUniformID("viewMatrix"), 1, GL_FALSE, camera->GetViewMatrix());


	glUniform3fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetUniformID("lightPos"), 1, lightPos);

	glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong")->GetProgram());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong")->GetUniformID("projectionMatrix"), 1, GL_FALSE, camera->GetProjectionMatrix());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong")->GetUniformID("viewMatrix"), 1, GL_FALSE, camera->GetViewMatrix());



	glUniform3fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong")->GetUniformID("lightPos"), 1, lightPos);
	
	sceneGraph.Render();

	glUseProgram(0);
}

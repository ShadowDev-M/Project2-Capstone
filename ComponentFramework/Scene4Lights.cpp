#include <glew.h>
#include <iostream>
#include <SDL.h>
#include "Scene4Lights.h"
#include <MMath.h>
#include "Debug.h"
#include "ExampleXML.h"
#include "InputManager.h"
#include "imgui_stdlib.h"

using namespace ImGui;

void Scene4Lights::ShowSaveDialog()
{
	if (showSaveFileDialog) {
		ImGui::OpenPopup("Save File");
		showSaveFileDialog = false;
	}

	// sets the placement and size of the dialog box
	const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x - 200, mainViewport->WorkPos.y - 200), ImGuiCond_Appearing);
	ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Appearing);

	if (ImGui::BeginPopupModal("Save File", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Enter The Name of The File You Want to Save:");
		ImGui::InputText("##NameOfSaveFile", &saveFileName);
		ImGui::Separator();

		if (ImGui::Button("Save File ##Button")) {
			sceneGraph.SaveFile(saveFileName);
			saveFileName.clear();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			saveFileName.clear();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void Scene4Lights::ShowLoadDialog()
{
	if (showLoadFileDialog) {
		ImGui::OpenPopup("Load File");
		showLoadFileDialog = false;
	}

	// sets the placement and size of the dialog box
	const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x - 200, mainViewport->WorkPos.y - 200), ImGuiCond_Appearing);
	ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Appearing);

	if (ImGui::BeginPopupModal("Load File", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Enter The Name of The File You Want to Load:");
		ImGui::InputText("##NameOfLoadFile", &saveFileName);
		ImGui::Separator();

		if (ImGui::Button("Load File ##Button")) {
			sceneGraph.RemoveAllActors();
			XMLObjectFile::addActorsFromFile(&sceneGraph, saveFileName);

			camera = std::make_shared<CameraActor>(nullptr, 45.0f, (16.0f / 9.0f), 0.5f, 100.0f);

			camera->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, 40.0f), QMath::inverse(Quaternion()));

			camera->OnCreate();

			sceneGraph.AddActor(camera);

			camera->fixCameraToTransform();

			sceneGraph.OnCreate();
			saveFileName.clear();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			saveFileName.clear();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

Scene4Lights::Scene4Lights() : drawInWireMode{ false } {
	Debug::Info("Created Scene4Lights: ", __FILE__, __LINE__);
}

Scene4Lights::~Scene4Lights() {
	Debug::Info("Deleted Scene4Lights: ", __FILE__, __LINE__);
}

bool Scene4Lights::OnCreate() {
	Debug::Info("Loading assets Scene4Lights: ", __FILE__, __LINE__);

	AssetManager::getInstance().OnCreate();
	
	AssetManager::getInstance().ListAllAssets();

	camera = std::make_shared<CameraActor>(nullptr, 45.0f, (16.0f / 9.0f), 0.5f, 100.0f);
	
	camera->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, 40.0f), QMath::inverse( Quaternion()));
	
	camera->OnCreate();
	
	sceneGraph.AddActor(camera);

	//example.readDoc();

	camera->fixCameraToTransform();
	
	LightActor* light1 = new LightActor(nullptr,
		Vec3(1.0f, 2.0f, -10.0f),
		Vec4(0.0f, 1.0f, 1.0f, 1.0f),
		Vec4(0.4f, 0.4f, 0.4f, 1.0f),
		40.0f);

	LightActor* light2 = new LightActor(nullptr,
		Vec3(-1.0f, -2.0f, -10.0f),
		Vec4(1.0f, 0.0f, 0.0f, 1.0f),
		Vec4(0.4f, 0.4f, 0.4f, 1.0f),
		40.0f);

	// Light Pos
	lights.push_back(light1);
	lights.push_back(light2);

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


void Scene4Lights::OnDestroy() {
	Debug::Info("Deleting assets Scene4Lights: ", __FILE__, __LINE__);

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
	for (int i = 0; i < lights.size(); ++i) {
		lights[i]->OnDestroy();
		delete lights[i];
	}


}

void Scene4Lights::HandleEvents(const SDL_Event& sdlEvent) {

	InputManager::getInstance().HandleEvents(sdlEvent, &sceneGraph, &collisionSystem);

	
}


void Scene4Lights::Update(const float deltaTime) {

	InputManager::getInstance().update(deltaTime, &sceneGraph);
	
	collisionSystem.Update(deltaTime);

	sceneGraph.Update(deltaTime);
}

void Scene4Lights::Render() const {
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

	if (showDemoWindow) {
		ImGui::ShowDemoWindow(&showDemoWindow);
	}
			
	if (showHierarchyWindow) {
		hierarchyWindow->ShowHierarchyWindow(&showHierarchyWindow);
	}

	if (showInspectorWindow) {
		inspectorWindow->ShowInspectorWindow(&showInspectorWindow);
	}

	if (showAssetmanagerWindow) {
		assetManagerWindow->ShowAssetManagerWindow(&showAssetmanagerWindow);
	}

	if (BeginMainMenuBar()) {
		if (BeginMenu("File")) {
			if (MenuItem("Save File ##MenuItem", "Ctrl+S")) {
				showSaveFileDialog = true;
				
			}
			if (MenuItem("Load File ##MenuItem", "Ctrl+L")) {
				showLoadFileDialog = true;
			}

			EndMenu();
		}

		if (BeginMenu("Windows")) {
			MenuItem("Demo", nullptr, &showDemoWindow);
			MenuItem("Hierarchy", nullptr, &showHierarchyWindow);
			MenuItem("Inspector", nullptr, &showInspectorWindow);
			MenuItem("Asset Manager", nullptr, &showAssetmanagerWindow);

			EndMenu();
		}
		EndMainMenuBar();
	}
	
	const_cast<Scene4Lights*>(this)->ShowSaveDialog();
	const_cast<Scene4Lights*>(this)->ShowLoadDialog();

	// Rendering
	ImGui::Render();
	glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
	glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	

	glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetProgram());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetUniformID("projectionMatrix"), 1, GL_FALSE, camera->GetProjectionMatrix());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetUniformID("viewMatrix"), 1, GL_FALSE, camera->GetViewMatrix());

	for (int i = 0; i < lights.size(); ++i) {
		glUniform3fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetUniformID("pos"), 1, lights[i]->getPos());
	}

	//glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_MultiPhong")->GetProgram());
	//glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_MultiPhong")->GetUniformID("projectionMatrix"), 1, GL_FALSE, camera->GetProjectionMatrix());
	//glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_MultiPhong")->GetUniformID("viewMatrix"), 1, GL_FALSE, camera->GetViewMatrix());

	//for (int i = 0; i < lights.size(); ++i) {
	//	glUniform3fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_MultiPhong")->GetUniformID("lightPos"), i + 1, lights[i]->getPos());
	//	glUniform4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_MultiPhong")->GetUniformID("diffuse"), i + 1, lights[i]->getDiff());
	//	glUniform4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_MultiPhong")->GetUniformID("specular"), i + 1, lights[i]->getSpec());
	//	glUniform1f(AssetManager::getInstance().GetAsset<ShaderComponent>("S_MultiPhong")->GetUniformID("intensity"), lights[i]->getIntensity());
	//}
	//glUniform4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_MultiPhong")->GetUniformID("ambient"), 1, Vec4(0.1f, 0.1f, 0.1f, 1.0f));

	//glUniform1ui(AssetManager::getInstance().GetAsset<ShaderComponent>("S_MultiPhong")->GetUniformID("numLights"), lights.size());


	
	
	sceneGraph.Render();

	glUseProgram(0);
}

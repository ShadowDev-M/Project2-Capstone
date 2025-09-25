#include <glew.h>
#include <iostream>
#include <SDL.h>
#include "Scene3GUI.h"
#include <MMath.h>
#include "Debug.h"
#include "ExampleXML.h"
#include "InputManager.h"
#include "imgui_stdlib.h"
#include "CameraComponent.h"
#include <filesystem>
using namespace ImGui;

void Scene3GUI::ShowSaveDialog()
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
		ImGui::InputText("##NameOfSaveFile", &SceneGraph::getInstance().cellFileName);
		ImGui::Separator();

		if (ImGui::Button("Save File ##Button")) {
			std::filesystem::create_directory("Game Objects/" + SceneGraph::getInstance().cellFileName);
			std::cout << SceneGraph::getInstance().cellFileName;
			
			SceneGraph::getInstance().SaveFile(SceneGraph::getInstance().cellFileName);
			SceneGraph::getInstance().cellFileName.clear();
			ImGui::CloseCurrentPopup();
		}



		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			SceneGraph::getInstance().cellFileName.clear();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void Scene3GUI::ShowLoadDialog()
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
		ImGui::InputText("##NameOfLoadFile", &SceneGraph::getInstance().cellFileName);
		ImGui::Separator();

		if (ImGui::Button("Load File ##Button")) {
			SceneGraph::getInstance().RemoveAllActors();
			XMLObjectFile::addActorsFromFile(&SceneGraph::getInstance(), SceneGraph::getInstance().cellFileName);

			//Ref<Actor> cameraActor = std::make_shared<Actor>(nullptr, "cameraActor");
			//cameraActor->AddComponent<CameraComponent>(cameraActor, 45.0f, (16.0f / 9.0f), 0.5f, 100.0f);
			//cameraActor->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, 40.0f), QMath::inverse(Quaternion()));

			//cameraActor->OnCreate();

			//sceneGraph.AddActor(cameraActor);

			//sceneGraph.setUsedCamera(cameraActor->GetComponent<CameraComponent>());

			//cameraActor->GetComponent<CameraComponent>()->fixCameraToTransform();

			////Create second camera as a test


			//Ref<Actor> cameraActorTwo = std::make_shared<Actor>(nullptr, "cameraActor2");
			//cameraActorTwo->AddComponent<CameraComponent>(cameraActorTwo, 45.0f, (16.0f / 9.0f), 0.5f, 100.0f);
			//cameraActorTwo->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, 40.0f), QMath::inverse(Quaternion()));

			//cameraActorTwo->OnCreate();

			//sceneGraph.AddActor(cameraActorTwo);

			//sceneGraph.setUsedCamera(cameraActorTwo->GetComponent<CameraComponent>());


			//cameraActorTwo->GetComponent<CameraComponent>()->fixCameraToTransform();

			SceneGraph::getInstance().setUsedCamera(0);
			SceneGraph::getInstance().checkValidCamera();



			SceneGraph::getInstance().OnCreate();
			SceneGraph::getInstance().cellFileName.clear();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			SceneGraph::getInstance().cellFileName.clear();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

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

	

	//camera = std::make_shared<CameraActor>(nullptr, 45.0f, (16.0f / 9.0f), 0.5f, 100.0f);
	////
	//camera->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, 40.0f), QMath::inverse( Quaternion()));
	////
	//camera->OnCreate();
	//
	//sceneGraph.AddActor(camera);

	/*Ref<Actor> cameraActor = std::make_shared<Actor>(nullptr, "cameraActor");
	cameraActor->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, 40.0f), QMath::inverse(Quaternion()));
	cameraActor->OnCreate();
	sceneGraph.AddActor(cameraActor);

	cameraActor->AddComponent<CameraComponent>(cameraActor, 45.0f, (16.0f / 9.0f), 0.5f, 100.0f);
	cameraActor->GetComponent<CameraComponent>()->OnCreate();
	cameraActor->GetComponent<CameraComponent>()->fixCameraToTransform();

	sceneGraph.setUsedCamera(cameraActor->GetComponent<CameraComponent>());*/
	
	SceneGraph::getInstance().checkValidCamera();
	
	//example.readDoc();

	//camera->fixCameraToTransform();

	//Ref<Actor> cameraActorTwo = std::make_shared<Actor>(nullptr, "cameraActor2");
	//cameraActorTwo->AddComponent<CameraComponent>(cameraActorTwo, 45.0f, (16.0f / 9.0f), 0.5f, 100.0f);
	//cameraActorTwo->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, 40.0f), QMath::inverse(Quaternion()));

	//cameraActorTwo->OnCreate();

	//sceneGraph.AddActor(cameraActorTwo);

	////sceneGraph.setUsedCamera(cameraActorTwo->GetComponent<CameraComponent>());


	//cameraActorTwo->GetComponent<CameraComponent>()->fixCameraToTransform();
	
	// Light Pos
	lightPos = Vec3(1.0f, 2.0f, -10.0f);

	// Board setup

	//sceneGraph.LoadActor("Board");




	//sceneGraph.LoadActor("Mario", sceneGraph.GetActor("Board"));

	
	

	//sceneGraph.LoadActor("Sphere", sceneGraph.GetActor("Board"));

	


	SceneGraph::getInstance().OnCreate();

	SceneGraph::getInstance().ListAllActors();

	//sceneGraph.RemoveActor("Sphere");
//	camera->fixCameraToTransform();
	XMLObjectFile::addActorsFromFile(&SceneGraph::getInstance(), "LevelThree");

	// pass along the scene graph to the windows
	hierarchyWindow = std::make_unique<HierarchyWindow>(&SceneGraph::getInstance());
	inspectorWindow = std::make_unique<InspectorWindow>(&SceneGraph::getInstance());
	assetManagerWindow = std::make_unique<AssetManagerWindow>(&SceneGraph::getInstance());


	return true;
}


void Scene3GUI::OnDestroy() {
	Debug::Info("Deleting assets Scene3GUI: ", __FILE__, __LINE__);

	// save all the assets in the assetmanager to the xml file then remove them all locally
	AssetManager::getInstance().SaveAssetDatabaseXML();
	AssetManager::getInstance().RemoveAllAssets();

	SceneGraph::getInstance().RemoveAllActors();

	//camera->OnDestroy();

	if (hierarchyWindow) {
		hierarchyWindow->ClearFilter();
	}
	if (assetManagerWindow) {
		assetManagerWindow->ClearFilter();
	}

}

void Scene3GUI::HandleEvents(const SDL_Event& sdlEvent) {

	//sceneGraph.checkValidCamera();

	InputManager::getInstance().HandleEvents(sdlEvent, &SceneGraph::getInstance(), &collisionSystem);

}


void Scene3GUI::Update(const float deltaTime) {

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

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	//_DEBUG
	// toolset

	//ImGui::DockSpaceOverViewport(ImGuiDockNodeFlags_PassthruCentralNode);

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
		if (BeginMenu("Tools")) {
			if (MenuItem("Change Camera to Default ##MenuItem", "Ctrl+M")) {
				SceneGraph::getInstance().useDebugCamera();
			}
			EndMenu();
		}

		EndMainMenuBar();
	}
	
	const_cast<Scene3GUI*>(this)->ShowSaveDialog();
	const_cast<Scene3GUI*>(this)->ShowLoadDialog();

	// Rendering
	ImGui::Render();
	glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
	glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	
	
	glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetProgram());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetUniformID("projectionMatrix"), 1, GL_FALSE, SceneGraph::getInstance().getUsedCamera()->GetProjectionMatrix());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetUniformID("viewMatrix"), 1, GL_FALSE, SceneGraph::getInstance().getUsedCamera()->GetViewMatrix());


	glUniform3fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline")->GetUniformID("lightPos"), 1, lightPos);

	glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong")->GetProgram());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong")->GetUniformID("projectionMatrix"), 1, GL_FALSE, SceneGraph::getInstance().getUsedCamera()->GetProjectionMatrix());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong")->GetUniformID("viewMatrix"), 1, GL_FALSE, SceneGraph::getInstance().getUsedCamera()->GetViewMatrix());



	glUniform3fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong")->GetUniformID("lightPos"), 1, lightPos);
	
	SceneGraph::getInstance().Render();

	glUseProgram(0);
}

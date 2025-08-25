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
	const Uint8* keys = SDL_GetKeyboardState(NULL);



	static bool mouseHeld = false;
	static int lastX = 0, lastY = 0;
	switch (sdlEvent.type) {
	case SDL_KEYDOWN:
		switch (sdlEvent.key.keysym.scancode) {
		case SDL_SCANCODE_T:
			drawInWireMode = !drawInWireMode;
			break;

		case SDL_SCANCODE_O:
			if (debugMoveSpeed <= 0.25f) { debugMoveSpeed /= 2.0f; }
			else {
				debugMoveSpeed -= 0.25f;

			}
			std::cout << "debugMoveSpeed = " << debugMoveSpeed << std::endl;
			break;

		case SDL_SCANCODE_P:
			if (debugMoveSpeed <= 0.25f) { debugMoveSpeed *= 2.0f; }
			else {
				debugMoveSpeed += 0.25f;

			}			
			std::cout << "debugMoveSpeed = " << debugMoveSpeed << std::endl;
			break;

		}
		
		break;

	case SDL_MOUSEBUTTONDOWN:
		if (sdlEvent.button.button == SDL_BUTTON_RIGHT) {
			/*mouseHeld = true;
			lastX = sdlEvent.button.x;
			lastY = sdlEvent.button.y;*/
		}

		if (sdlEvent.button.button == SDL_BUTTON_LEFT) {
			//mouseHeld = true;
			//lastX = sdlEvent.button.x;
			//lastY = sdlEvent.button.y;

			//
			//Vec3 startPos = camera->GetComponent<TransformComponent>()->GetPosition();
			//Vec3 endPos = startPos + rayCast(0, 0, camera->GetProjectionMatrix(), camera->GetViewMatrix()) * 20;
			mouseHeld = true;
			lastX = sdlEvent.button.x;
			lastY = sdlEvent.button.y;
		}


		

		break;

	case SDL_MOUSEBUTTONUP:
		if (sdlEvent.button.button == SDL_BUTTON_RIGHT) {
			mouseHeld = false;
		}
		if (sdlEvent.button.button == SDL_BUTTON_LEFT) {

			// makes it so ImGui handles the mouse click
			if (GetIO().WantCaptureMouse) {
				mouseHeld = false;
				return;
			}

			lastX = sdlEvent.button.x;
			lastY = sdlEvent.button.y;

			
			Vec3 startPos = camera->GetComponent<TransformComponent>()->GetPosition();
			Vec3 endPos = startPos + Raycast::screenRayCast(lastX, lastY, camera->GetProjectionMatrix(), camera->GetViewMatrix());

			
			//prepare for unintelligible logic for selecting 
			Ref<Actor> raycastedActor = collisionSystem.PhysicsRaycast(startPos, endPos);

			//(Slightly) more expensive debug selector if nothing was selected with the ColliderComponent PhysicsRaycast
			if (!raycastedActor) raycastedActor = sceneGraph.MeshRaycast(startPos, endPos);

			//an object was clicked
			if (raycastedActor) { 


				if (!keys[SDL_SCANCODE_LCTRL] && !(sceneGraph.debugSelectedAssets.find(raycastedActor->getActorName()) != sceneGraph.debugSelectedAssets.end())) { sceneGraph.debugSelectedAssets.clear(); }

				if (sceneGraph.debugSelectedAssets.find(raycastedActor->getActorName()) != sceneGraph.debugSelectedAssets.end() && keys[SDL_SCANCODE_LCTRL]) { sceneGraph.debugSelectedAssets.erase(raycastedActor->getActorName()); }

				else sceneGraph.debugSelectedAssets.emplace(raycastedActor->getActorName(), raycastedActor); 

			}
			//no object was clicked, and left control isn't pressed (making sure the user didn't accidentally misclicked during multi object selection before clearing selection)
			else if(!keys[SDL_SCANCODE_LCTRL])sceneGraph.debugSelectedAssets.clear();
			


			//startPos.print();
			//endPos.print();
			mouseHeld = false;

		}
		break;

	case SDL_MOUSEMOTION:
		if (mouseHeld) {

			// makes it so ImGui handles the mouse motion
			if (GetIO().WantCaptureMouse) {
				mouseHeld = false;
				return;
			}

			if (!(sceneGraph.debugSelectedAssets.empty())) {
			//get direction vector of new vector of movement from old mouse pos and new mouse pos
			
			int deltaX = sdlEvent.motion.x - lastX;
			int deltaY = sdlEvent.motion.y - lastY;
			lastX = sdlEvent.motion.x;
			lastY = sdlEvent.motion.y;

			auto& debugGraph = sceneGraph.debugSelectedAssets;


			for (const auto& obj  : debugGraph) {
				Ref<TransformComponent> transform = obj.second->GetComponent<TransformComponent>();
				Vec3 vectorMove = transform->GetPosition() + Vec3(deltaX, -deltaY, transform->GetPosition().z) * (camera->GetComponent<TransformComponent>()->GetPosition().z - transform->GetPosition().z) / 40 * 0.045;
				transform->SetPos(vectorMove.x, vectorMove.y, transform->GetPosition().z);

			}
			//transform->GetPosition() + Vec3(deltaX, deltaY, transform->GetPosition().z);
				//apply vector to asset
			 }

			//int deltaX = sdlEvent.motion.x - lastX;
			//int deltaY = sdlEvent.motion.y - lastY;
			//lastX = sdlEvent.motion.x;
			//lastY = sdlEvent.motion.y;

			//float sensitivity = 0.005f;
			//auto transform = camera->GetComponent<TransformComponent>();
			//Quaternion currentRotation = transform->GetQuaternion();

			//// Get local axes
			//Vec3 right = QMath::rotate( Vec3(-1, 0, 0), currentRotation);  
			//Vec3 up = QMath::rotate( Vec3(0, -1, 0),currentRotation);     

			//// Create rotations around the local axes
			//Quaternion pitchRotation = QMath::normalize(Quaternion(1.0f, right * (deltaY * sensitivity)));
			//Quaternion yawRotation = QMath::normalize(Quaternion(1.0f, up * (deltaX * sensitivity)));

			//// Apply rotations: yaw then pitch
			//Quaternion newRotation = QMath::normalize(yawRotation * pitchRotation * currentRotation);


			//camera->GetComponent<TransformComponent>()->SetTransform(
			//	camera->GetComponent<TransformComponent>()->GetPosition(),
			//	newRotation
			//);
			//camera->fixCameraToTransform();


		}
		break;
	default:
		break;
	}
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

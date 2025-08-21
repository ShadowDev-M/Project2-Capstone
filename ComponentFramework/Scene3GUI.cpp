#include <glew.h>
#include <iostream>
#include <SDL.h>
#include "Scene3GUI.h"
#include <MMath.h>
#include "Debug.h"
#include "ExampleXML.h"

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
	/*TransformComponent* example = new TransformComponent(nullptr, Vec3(0.0f, 0.0f, -15.0f), Quaternion(), Vec3(1.0f, 1.0f, 1.0f));
	

	XMLObjectFile::writeActor("Bob");
	XMLObjectFile::writeActor("Bill");

	XMLObjectFile::writeComponent<TransformComponent>("Bob", example);
	XMLObjectFile::writeComponent<TransformComponent>("Bill", example);


	TransformComponent* tempTestWrite = std::apply([](auto&&... args) {
		return new TransformComponent(args...);
		}, XMLObjectFile::getComponent<TransformComponent>("Bob"));

	

	XMLObjectFile::writeCellFile("LevelOne");
	XMLObjectFile::writeActorToCell("LevelOne", "Bob", true);
	XMLObjectFile::writeActorToCell("LevelOne", "Bill", true);
	*/

	/*TransformComponent* exampleTransform = new TransformComponent(nullptr, Vec3(0.0f, 0.0f, 0.0f), Quaternion());

	XMLObjectFile::writeActor("Jeff");
	XMLObjectFile::writeComponent<TransformComponent>("Jeff", exampleTransform);

	XMLObjectFile::writeCellFile("LevelTwo");
	XMLObjectFile::writeActorToCell("LevelTwo", "Jeff", true);*/



	/*std::cout <<
		"position: Vec3("
		<< tempTestWrite->GetPosition().x << " "
		<< tempTestWrite->GetPosition().y << " "
		<< tempTestWrite->GetPosition().z << ")"
		<< std::endl;
	std::cout << "Rotation: " <<
		tempTestWrite->GetQuaternion().w << ", Vec3("
		<< tempTestWrite->GetQuaternion().ijk.x << " "
		<< tempTestWrite->GetQuaternion().ijk.y << " "
		<< tempTestWrite->GetQuaternion().ijk.z << ")"
		<< std::endl;
	std::cout <<
		"scale: Vec3("
		<< tempTestWrite->GetScale().x << " "
		<< tempTestWrite->GetScale().y << " "
		<< tempTestWrite->GetScale().z << ")"
		<< std::endl;*/

	// Light Pos
	lightPos = Vec3(1.0f, 2.0f, -10.0f);

	// Board setup

	sceneGraph.LoadActor("Board");


	/*Ref<Actor> board = std::make_shared<Actor>(nullptr, "Board");
	board->AddComponent(AssetManager::getInstance().GetAsset<MeshComponent>("SM_Plane"));
	board->AddComponent(AssetManager::getInstance().GetAsset<MaterialComponent>("M_ChessBoard"));
	board->AddComponent(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong"));
	board->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, 0.0f), Quaternion(0.0f, Vec3(0.0f, 0.0f, 0.0f)), Vec3(1.0f, 1.0f, 1.0f));
	board->OnCreate();
	sceneGraph.AddActor(board);*/

	// mario setup

	sceneGraph.LoadActor("Mario", sceneGraph.GetActor("Board"));

	
	/*Ref<Actor> mario = std::make_shared<Actor>(board.get(), "Mario");
	mario->AddComponent(AssetManager::getInstance().GetAsset<MeshComponent>("SM_Mario"));
	mario->AddComponent(AssetManager::getInstance().GetAsset<MaterialComponent>("M_MarioN"));
	mario->AddComponent(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong"));
	mario->AddComponent<TransformComponent>(nullptr, Vec3(2.4f, -0.5f, 0.0f), QMath::toQuaternion(MMath::rotate(90.0f, Vec3(1.0f, 0.0f, 0.0f))), Vec3(1.25f, 1.25f, 1.25f));
	mario->AddComponent<PhysicsComponent>(nullptr, 1.0f);
	mario->AddComponent<CollisionComponent>(nullptr, 0.6f);
	mario->GetComponent<PhysicsComponent>()->setVel(Vec3(0.0f, 0, 0.0f));
	collisionSystem.AddActor(mario);
	mario->OnCreate();
	sceneGraph.AddActor(mario);*/

	// sphere setup
	//sphere->AddComponent(AssetManager::getInstance().GetAsset<MeshComponent>("SM_Sphere"));
//	sphere->AddComponent(AssetManager::getInstance().GetAsset<MaterialComponent>("M_Sphere"));
	//sphere->AddComponent(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong"));
	//sphere->AddComponent<TransformComponent>(nullptr, Vec3(2.4f, -4.5f, 0.0f), QMath::toQuaternion(MMath::rotate(90.0f, Vec3(1.0f, 0.0f, 0.0f))), Vec3(1.5f, 1.5f, 1.5f));

	//Ref<Actor> sphere = std::make_shared<Actor>(nullptr, "Sphere");

	sceneGraph.LoadActor("Sphere", sceneGraph.GetActor("Board"));



	//sphere->AddComponent<MaterialComponent>(AssetManager::getInstance().GetAsset<MaterialComponent>(XMLObjectFile::getComponent<MaterialComponent>("Sphere")));
	//sphere->AddComponent<ShaderComponent>(AssetManager::getInstance().GetAsset<ShaderComponent>(XMLObjectFile::getComponent<ShaderComponent>("Sphere")));
	//sphere->AddComponent<MeshComponent>(AssetManager::getInstance().GetAsset<MeshComponent>(XMLObjectFile::getComponent<MeshComponent>("Sphere")));
	//sphere->AddComponent<TransformComponent>(std::apply([](auto&&... args) {
	//	return new TransformComponent(args...);
	//	}, XMLObjectFile::getComponent<TransformComponent>("Sphere")));


	//Ref<MeshComponent> test = XMLObjectFile::getComponent<MeshComponent>("Sphere");


	//sphere->AddComponent<PhysicsComponent>(nullptr, 2.0f);
	//sphere->AddComponent<CollisionComponent>(nullptr, 0.75f);
	//sphere->GetComponent<PhysicsComponent>()->setVel(Vec3(0.0f, 3.0f, 0.0f));
	//collisionSystem.AddActor(sphere);
	//sphere->OnCreate();
	//sceneGraph.AddActor(sphere);

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



	static float angle = 0.0f;
	angle += 20.0f * deltaTime;


	// makes board spin
	//Matrix4 boardModel;
	bool keyPressed = false;
	const Uint8* keys = SDL_GetKeyboardState(NULL);

	Vec3 inputVector = Vec3();

	if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) {
		inputVector += Vec3(0, 1, 0);
		keyPressed = true;
	}

	if (keys[SDL_SCANCODE_S] && keys[SDL_SCANCODE_LCTRL] && !keyPressed) {
		keyPressed = true;
		sceneGraph.SaveFile("LevelThree");

		

	}
	else if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) {
		inputVector += Vec3(0, -1, 0);
		keyPressed = true;
	}

	if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) {
		
		inputVector += Vec3(-1, 0, 0);
		keyPressed = true;

	}
	if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) {
		
		inputVector += Vec3(1, 0, 0);
		keyPressed = true;

	}
	if (keys[SDL_SCANCODE_R]) {

		inputVector += Vec3(0, 0, 1);
		keyPressed = true;

	}
	if (keys[SDL_SCANCODE_E]) {

		inputVector += Vec3(0, 0, -1);
		keyPressed = true;
	}
	if (keys[SDL_SCANCODE_F]) {

		//sceneGraph.debugSelectedAssets.clear();
	}
	if (keys[SDL_SCANCODE_M]) {

		//just use f3
		
		// XMLObjectFile::addActorsFromFile(&sceneGraph, "LevelThree");
	}

	if (keyPressed) {
		Quaternion q = camera->GetComponent<TransformComponent>()->GetQuaternion();
		
		//inputVector.print();
		
		Quaternion rotation = (QMath::normalize(q));
		camera->GetComponent<TransformComponent>()->GetPosition().print();


		
		//convert local direction into world coords 
		Vec3 worldForward = QMath::rotate(inputVector, rotation) * debugMoveSpeed;

		if (!(sceneGraph.debugSelectedAssets.empty())) {
			//auto& debugGraph = sceneGraph.debugSelectedAssets;


			for (const auto& obj : sceneGraph.debugSelectedAssets) {

				obj.second->GetComponent<TransformComponent>()->SetTransform(
					obj.second->GetComponent<TransformComponent>()->GetPosition() + worldForward,
					obj.second->GetComponent<TransformComponent>()->GetQuaternion(), 
					obj.second->GetComponent<TransformComponent>()->GetScale()
				);
			}
		}
		else {
			camera->GetComponent<TransformComponent>()->SetTransform(
				camera->GetComponent<TransformComponent>()->GetPosition() + worldForward,
				q
			);
			camera->fixCameraToTransform();
		}

	}
	if (keys[SDL_SCANCODE_SPACE]) {
		//sceneGraph.GetActor("Mario")->GetComponent<TransformComponent>()->SetTransform(camera->GetComponent<TransformComponent>()->GetPosition(), (camera->GetComponent<TransformComponent>()->GetQuaternion()));


		

		
	}

	//if (boardModel) sceneGraph.GetActor("Board")->GetComponent<TransformComponent>()->SetOrientation(QMath::toQuaternion(boardModel));
	
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

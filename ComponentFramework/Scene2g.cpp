#include <glew.h>
#include <iostream>
#include <SDL.h>
#include "Scene2g.h"
#include <MMath.h>
#include "Debug.h"
#include "ExampleXML.h"
Scene2g::Scene2g() : drawInWireMode{ false } {
	Debug::Info("Created Scene2g: ", __FILE__, __LINE__);
}

Scene2g::~Scene2g() {
	Debug::Info("Deleted Scene2g: ", __FILE__, __LINE__);
}

bool Scene2g::OnCreate() {
	Debug::Info("Loading assets Scene2g: ", __FILE__, __LINE__);

	AssetManager::getInstance().OnCreate();
	
	AssetManager::getInstance().ListAllAssets();

	camera = std::make_shared<CameraActor>(nullptr, 45.0f, (16.0f / 9.0f), 0.5f, 100.0f);
	
	camera->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, 40.0f), QMath::inverse( Quaternion()));
	
	camera->OnCreate();
	
	std::cout << sceneGraph.AddActor(camera) << std::endl;

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

	XMLObjectFile::addActorsFromFile(&sceneGraph, "LevelOne");


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
	Ref<Actor> board = std::make_shared<Actor>(nullptr, "Board");
	board->AddComponent(AssetManager::getInstance().GetAsset<MeshComponent>("SM_Plane"));
	board->AddComponent(AssetManager::getInstance().GetAsset<MaterialComponent>("M_ChessBoard"));
	board->AddComponent(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong"));
	board->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, 0.0f), Quaternion(0.0f, Vec3(0.0f, 0.0f, 0.0f)), Vec3(1.0f, 1.0f, 1.0f));
	board->OnCreate();
	sceneGraph.AddActor(board);

	// mario setup
	Ref<Actor> mario = std::make_shared<Actor>(board.get(), "Mario");
	mario->AddComponent(AssetManager::getInstance().GetAsset<MeshComponent>("SM_Mario"));
	mario->AddComponent(AssetManager::getInstance().GetAsset<MaterialComponent>("M_MarioN"));
	mario->AddComponent(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong"));
	mario->AddComponent<TransformComponent>(nullptr, Vec3(2.4f, -0.5f, 0.0f), QMath::toQuaternion(MMath::rotate(90.0f, Vec3(1.0f, 0.0f, 0.0f))), Vec3(1.25f, 1.25f, 1.25f));
	mario->AddComponent<PhysicsComponent>(nullptr, 1.0f);
	mario->AddComponent<CollisionComponent>(nullptr, 0.6f);
	mario->GetComponent<PhysicsComponent>()->setVel(Vec3(0.0f, 0, 0.0f));
	collisionSystem.AddActor(mario);
	mario->OnCreate();
	sceneGraph.AddActor(mario);

	// sphere setup
	Ref<Actor> sphere = std::make_shared<Actor>(board.get(), "Sphere");
	sphere->AddComponent(AssetManager::getInstance().GetAsset<MeshComponent>("SM_Sphere"));
	sphere->AddComponent(AssetManager::getInstance().GetAsset<MaterialComponent>("M_Sphere"));
	sphere->AddComponent(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong"));
	sphere->AddComponent<TransformComponent>(nullptr, Vec3(2.4f, -4.5f, 0.0f), QMath::toQuaternion(MMath::rotate(90.0f, Vec3(1.0f, 0.0f, 0.0f))), Vec3(1.5f, 1.5f, 1.5f));
	//sphere->AddComponent<PhysicsComponent>(nullptr, 2.0f);
	//sphere->AddComponent<CollisionComponent>(nullptr, 0.75f);
	//sphere->GetComponent<PhysicsComponent>()->setVel(Vec3(0.0f, 3.0f, 0.0f));
	//collisionSystem.AddActor(sphere);
	sphere->OnCreate();
	sceneGraph.AddActor(sphere);

	sceneGraph.OnCreate();

	sceneGraph.ListAllActors();

	//sceneGraph.RemoveActor("Sphere");
	camera->fixCameraToTransform();
	
	return true;
}

Vec3 rayCast(double xpos, double ypos, Matrix4 projection, Matrix4 view) {
	// converts a position from the 2d xpos, ypos to a normalized 3d direction
	float x = (2.0f * xpos) / 1280 - 1.0f;
	float y = 1.0f - (2.0f * ypos) / 720;
	float z = 1.0f;
	Vec3 ray_nds = Vec3(x, y, z);
	Vec4 ray_clip = Vec4(ray_nds.x, ray_nds.y, -1.0f, 1.0f);
	// eye space to clip we would multiply by projection so
	// clip space to eye space is the inverse projection
	Vec4 ray_eye = MMath::inverse(projection) * ray_clip;
	// convert point to forwards
	ray_eye = Vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
	// world space to eye space is usually multiply by view so
	// eye space to world space is inverse view
	Vec4 inv_ray_wor = (MMath::inverse(view) * ray_eye);
	Vec3 ray_wor = Vec3(inv_ray_wor.x, inv_ray_wor.y, inv_ray_wor.z);
	ray_wor = VMath::normalize(ray_wor);
	return ray_wor;
}

void Scene2g::OnDestroy() {
	Debug::Info("Deleting assets Scene2g: ", __FILE__, __LINE__);

	AssetManager::getInstance().RemoveAllAssets();

	sceneGraph.RemoveAllActors();

	camera->OnDestroy();

}

void Scene2g::HandleEvents(const SDL_Event& sdlEvent) {
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
			mouseHeld = false;

			lastX = sdlEvent.button.x;
			lastY = sdlEvent.button.y;


			Vec3 startPos = camera->GetComponent<TransformComponent>()->GetPosition();
			Vec3 endPos = startPos + rayCast(lastX, lastY, camera->GetProjectionMatrix(), camera->GetViewMatrix()) * 100;

			Ref<Actor> raycastedActor = collisionSystem.PhysicsRaycast(startPos, endPos);

			if (raycastedActor)  selectedAsset = raycastedActor;
			else selectedAsset = nullptr;

			//startPos.print();
			//endPos.print();
		}
		break;

	case SDL_MOUSEMOTION:
		if (mouseHeld) {

			if (selectedAsset) {
			//get direction vector of new vector of movement from old mouse pos and new mouse pos
			
			int deltaX = sdlEvent.motion.x - lastX;
			int deltaY = sdlEvent.motion.y - lastY;
			lastX = sdlEvent.motion.x;
			lastY = sdlEvent.motion.y;

			Ref<TransformComponent> transform = selectedAsset->GetComponent<TransformComponent>();
			//transform->GetPosition() + Vec3(deltaX, deltaY, transform->GetPosition().z);
				//apply vector to asset
			Vec3 vectorMove = transform->GetPosition() + Vec3(deltaX, -deltaY, transform->GetPosition().z) * (camera->GetComponent<TransformComponent>()->GetPosition().z - transform->GetPosition().z) / 40 * 0.045;
			transform->SetPos(vectorMove.x, vectorMove.y, transform->GetPosition().z);
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

void Scene2g::Update(const float deltaTime) {



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
	if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) {
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

		selectedAsset = nullptr;
	}
	if (keyPressed) {
		Quaternion q = camera->GetComponent<TransformComponent>()->GetQuaternion();
		
		//inputVector.print();
		
		Quaternion rotation = (QMath::normalize(q));
		camera->GetComponent<TransformComponent>()->GetPosition().print();


		
		//convert local direction into world coords 
		Vec3 worldForward = QMath::rotate(inputVector, rotation) * debugMoveSpeed;

		if (selectedAsset) {
			selectedAsset->GetComponent<TransformComponent>()->SetTransform(
				selectedAsset->GetComponent<TransformComponent>()->GetPosition() + worldForward,
				selectedAsset->GetComponent<TransformComponent>()->GetQuaternion()
			);
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

void Scene2g::Render() const {
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

	glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong")->GetProgram());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong")->GetUniformID("projectionMatrix"), 1, GL_FALSE, camera->GetProjectionMatrix());
	glUniformMatrix4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong")->GetUniformID("viewMatrix"), 1, GL_FALSE, camera->GetViewMatrix());

	glUniform3fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong")->GetUniformID("lightPos"), 1, lightPos);

	sceneGraph.Render();

	glUseProgram(0);
}
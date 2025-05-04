#include <glew.h>
#include <iostream>
#include <SDL.h>
#include "Scene2g.h"
#include <MMath.h>
#include "Debug.h"

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

	camera = new CameraActor(nullptr, 45.0f, (16.0f / 9.0f), 0.5f, 100.0f);
	camera->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, -15.0f), Quaternion(), Vec3(1.0f, 1.0f, 1.0f));
	camera->OnCreate();

	// Light Pos
	lightPos = Vec3(5.0f, -1.0f, 0.0f);

	// Board setup
	Ref<Actor> board = std::make_shared<Actor>(nullptr, "Board");
	board->AddComponent(AssetManager::getInstance().GetAsset<MeshComponent>("SM_Plane"));
	board->AddComponent(AssetManager::getInstance().GetAsset<MaterialComponent>("M_ChessBoard"));
	board->AddComponent(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong"));
	board->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, 0.0f), Quaternion(0.0f, Vec3(0.0f, 1.0f, 0.0f)), Vec3(1.0f, 1.0f, 1.0f));
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
	mario->GetComponent<PhysicsComponent>()->setVel(Vec3(0.0f, 0.75f, 0.0f));
	collisionSystem.AddActor(mario);
	mario->OnCreate();
	sceneGraph.AddActor(mario);

	// sphere setup
	Ref<Actor> sphere = std::make_shared<Actor>(board.get(), "Sphere");
	sphere->AddComponent(AssetManager::getInstance().GetAsset<MeshComponent>("SM_Sphere"));
	sphere->AddComponent(AssetManager::getInstance().GetAsset<MaterialComponent>("M_Sphere"));
	sphere->AddComponent(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong"));
	sphere->AddComponent<TransformComponent>(nullptr, Vec3(2.4f, -4.5f, 0.0f), QMath::toQuaternion(MMath::rotate(90.0f, Vec3(1.0f, 0.0f, 0.0f))), Vec3(1.5f, 1.5f, 1.5f));
	sphere->AddComponent<PhysicsComponent>(nullptr, 2.0f);
	sphere->AddComponent<CollisionComponent>(nullptr, 0.75f);
	sphere->GetComponent<PhysicsComponent>()->setVel(Vec3(0.0f, 3.0f, 0.0f));
	collisionSystem.AddActor(sphere);
	sphere->OnCreate();
	sceneGraph.AddActor(sphere);

	sceneGraph.OnCreate();

	sceneGraph.ListAllActors();

	//sceneGraph.RemoveActor("Sphere");

	return true;
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

		}
		break;

	case SDL_MOUSEBUTTONDOWN:
		if (sdlEvent.button.button == SDL_BUTTON_LEFT) {
			mouseHeld = true;
			lastX = sdlEvent.button.x;
			lastY = sdlEvent.button.y;
		}
		break;

	case SDL_MOUSEBUTTONUP:
		if (sdlEvent.button.button == SDL_BUTTON_LEFT) {
			mouseHeld = false;
		}
		break;

	case SDL_MOUSEMOTION:
		if (mouseHeld) {
			int deltaX = sdlEvent.motion.x - lastX;
			int deltaY = sdlEvent.motion.y - lastY;
			lastX = sdlEvent.motion.x;
			lastY = sdlEvent.motion.y;

			// Adjust rotation based on mouse movement
			float sensitivity = 0.005f;
			Quaternion pitchRotation = QMath::normalize(Quaternion(1.0f, Vec3(deltaY * sensitivity, 0, 0)));
			Quaternion yawRotation = QMath::normalize(Quaternion(1.0f, Vec3(0, deltaX * sensitivity, 0)));

			Quaternion newRotation = yawRotation * pitchRotation * sceneGraph.GetActor("Board")->GetComponent<TransformComponent>()->GetQuaternion();
			sceneGraph.GetActor("Board")->GetComponent<TransformComponent>()->SetTransform(
				sceneGraph.GetActor("Board")->GetComponent<TransformComponent>()->GetPosition(),
				newRotation
			);
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

	const Uint8* keys = SDL_GetKeyboardState(NULL);

	if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) {
		Ref<TransformComponent> objMoved = sceneGraph.GetActor("Board")->GetComponent<TransformComponent>();
		objMoved->SetTransform(objMoved->GetPosition(), objMoved->GetQuaternion() * QMath::normalize(Quaternion(1.0f, Vec3(0.01, 0, 0))));
	
	}
	if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) {
		Ref<TransformComponent> objMoved = sceneGraph.GetActor("Board")->GetComponent<TransformComponent>();
		objMoved->SetTransform(objMoved->GetPosition(), objMoved->GetQuaternion() * QMath::normalize(Quaternion(1.0f, Vec3(-0.01, 0, 0))));

	}
	if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) {
		Ref<TransformComponent> objMoved = sceneGraph.GetActor("Board")->GetComponent<TransformComponent>();
		objMoved->SetTransform(objMoved->GetPosition(), objMoved->GetQuaternion() * QMath::normalize(Quaternion(1.0f, Vec3(0, -0.01, 0))));

	}
	if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) {
		Ref<TransformComponent> objMoved = sceneGraph.GetActor("Board")->GetComponent<TransformComponent>();
		objMoved->SetTransform(objMoved->GetPosition(), objMoved->GetQuaternion() * QMath::normalize(Quaternion(1.0f, Vec3(0, 0.01, 0))));


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
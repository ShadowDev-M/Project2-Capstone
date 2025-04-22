#include <glew.h>
#include <iostream>
#include <SDL.h>
#include "Scene1g.h"
#include <MMath.h>
#include "Debug.h"

Scene1g::Scene1g() : drawInWireMode{ false } {
	Debug::Info("Created Scene1g: ", __FILE__, __LINE__);
}

Scene1g::~Scene1g() {
	Debug::Info("Deleted Scene1g: ", __FILE__, __LINE__);
}

bool Scene1g::OnCreate() {
	Debug::Info("Loading assets Scene1g: ", __FILE__, __LINE__);

	
	// Setting up shared components
	phongShader = std::make_shared<ShaderComponent>(nullptr, "shaders/texturePhongVert.glsl", "shaders/texturePhongFrag.glsl");

	// Meshes
	pawnMesh = std::make_shared<MeshComponent>(nullptr, "meshes/Pawn.obj");
	bishopMesh = std::make_shared<MeshComponent>(nullptr, "meshes/Bishop.obj");
	kingMesh = std::make_shared<MeshComponent>(nullptr, "meshes/King.obj");
	knightMesh = std::make_shared<MeshComponent>(nullptr, "meshes/Knight.obj");
	queenMesh = std::make_shared<MeshComponent>(nullptr, "meshes/Queen.obj");
	rookMesh = std::make_shared<MeshComponent>(nullptr, "meshes/Rook.obj");
	planeMesh = std::make_shared<MeshComponent>(nullptr, "meshes/Plane.obj");

	// Materials
	blackChess = std::make_shared<MaterialComponent>(nullptr, "textures/Black Chess Base Colour.png");
	whiteChess = std::make_shared<MaterialComponent>(nullptr, "textures/White Chess Base Colour.png");
	chessBoard = std::make_shared<MaterialComponent>(nullptr, "textures/8x8_checkered_board.png");

	camera = new CameraActor(nullptr, 45.0f, (16.0f / 9.0f), 0.5f, 100.0f);
	camera->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, -15.0f), Quaternion(), Vec3(1.0f, 1.0f, 1.0f));
	camera->OnCreate();

	// Light Pos
	lightPos = Vec3(5.0f, -1.0f, 0.0f);

	// Chess board setup
	board = std::make_shared<Actor>(nullptr, "Board");
	board->AddComponent(planeMesh);
	board->AddComponent(chessBoard);
	board->AddComponent(phongShader);
	board->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, 0.0f), Quaternion(0.0f, Vec3(0.0f, 1.0f, 0.0f)), Vec3(1.0f, 1.0f, 1.0f));
	board->OnCreate();
	Actors.insert(std::pair(board->getActorName(), board));



	for (int i = 0; i < 2; i++) {
		// create key for specific chess piece (in this case the pawn), having "std::to_string(i)" allows me to access any specific actor I want after by just doing Actors.at("actorNX")
		std::string pawnN = "TestN" + std::to_string(i);
		// basic setup for the pawn, making it shared, parenting it, adding the shared components
		Ref<Actor> pawn = std::make_shared<Actor>(board.get());
		pawn->AddComponent(pawnMesh);
		pawn->AddComponent(phongShader);

		// first 8 are white pieces and are placed on one side
		
		pawn->AddComponent(whiteChess);
		// finding the exact postition for each piece was finicky, probably a better way to set this up
		pawn->AddComponent<TransformComponent>(nullptr, Vec3(-4.4f, -2.5f , 0.0f), QMath::toQuaternion(MMath::rotate(90.0f, Vec3(1.0f, 0.0f, 0.0f))), Vec3(0.25f, 0.25f, 0.25f));
		
		pawn->AddComponent<PhysicsComponent>(nullptr, 1.0f);
		pawn->AddComponent<CollisionComponent>(nullptr, 0.5f);
		collisionSystem.AddActor(pawn);

		pawn->OnCreate();
		// add the pawn to the unordered map, std::pair gives the key (pawn name) and the value (pawn actor)
		Actors.insert(std::pair(pawnN, pawn));
	}

	Actors.at("TestN0")->GetComponent<TransformComponent>()->SetPos(-4.4f, -1.0f, 0.0f);


	Actors.at("TestN0")->GetComponent<PhysicsComponent>()->setVel(Vec3(0.0f, 0.25f, 0.0f));
	Actors.at("TestN1")->GetComponent<PhysicsComponent>()->setVel(Vec3(0.0f, 0.5f, 0.0f));
	// a lot of loops... but it works
	// probably would look a lot better once the assetmanager is setup and I can create a proper scene graph

	for (int i = 0; i < 16; i++) {
		// create key for specific chess piece (in this case the pawn), having "std::to_string(i)" allows me to access any specific actor I want after by just doing Actors.at("actorNX")
		std::string pawnN = "pawnN" + std::to_string(i);
		// basic setup for the pawn, making it shared, parenting it, adding the shared components
		Ref<Actor> pawn = std::make_shared<Actor>(board.get());
		pawn->AddComponent(pawnMesh);
		pawn->AddComponent(phongShader);

		// first 8 are white pieces and are placed on one side
		if (i < 8) {
			pawn->AddComponent(whiteChess);
			// finding the exact postition for each piece was finicky, probably a better way to set this up
			pawn->AddComponent<TransformComponent>(nullptr, Vec3(-4.4f + i * 1.25f, -4.4f + 1.25f, 0.0f), QMath::toQuaternion(MMath::rotate(90.0f, Vec3(1.0f, 0.0f, 0.0f))), Vec3(0.25f, 0.25f, 0.25f));
		}
		// other 8 are black pieces and placed on the other side of the board
		else {
			pawn->AddComponent(blackChess);
			pawn->AddComponent<TransformComponent>(nullptr, Vec3(-4.4f + (i - 8) * 1.25f, -4.4f + 7.5f, 0.0f), QMath::toQuaternion(MMath::rotate(90.0f, Vec3(1.0f, 0.0f, 0.0f))), Vec3(0.25f, 0.25f, 0.25f));
		}

		pawn->OnCreate();
		// add the pawn to the unordered map, std::pair gives the key (pawn name) and the value (pawn actor)
		Actors.insert(std::pair(pawnN, pawn));
	}

	for (int i = 0; i < 4; i++) {
		std::string rookN = "rookN" + std::to_string(i);
		Ref<Actor> rook = std::make_shared<Actor>(board.get());
		rook->AddComponent(phongShader);
		rook->AddComponent(rookMesh);
			
		if (i < 2) {
			rook->AddComponent(whiteChess);	
			rook->AddComponent<TransformComponent>(nullptr, Vec3(-4.4f + i * 8.75f, -4.4f, 0.0f), QMath::toQuaternion(MMath::rotate(90.0f, Vec3(1.0f, 0.0f, 0.0f))), Vec3(0.25f, 0.25f, 0.25f));
		}
		else {
			rook->AddComponent(blackChess);	
			rook->AddComponent<TransformComponent>(nullptr, Vec3(-4.4f + (i - 2) * 8.75f, -4.4f + 8.75f, 0.0f), QMath::toQuaternion(MMath::rotate(90.0f, Vec3(1.0f, 0.0f, 0.0f))), Vec3(0.25f, 0.25f, 0.25f));
		}
		
		rook->OnCreate();
		Actors.insert(std::pair(rookN, rook));
	}

	for (int i = 0; i < 4; i++) {
		std::string knightN = "knightN" + std::to_string(i);
		Ref<Actor> knight = std::make_shared<Actor>(board.get());
		knight->AddComponent(phongShader);
		knight->AddComponent(knightMesh);

		if (i < 2) {
			knight->AddComponent(whiteChess);
			knight->AddComponent<TransformComponent>(nullptr, Vec3((-4.4f + 1.25f) + i * 6.25f, -4.4f, 0.0f), QMath::toQuaternion(MMath::rotate(90.0f, Vec3(1.0f, 0.0f, 0.0f))) * QMath::toQuaternion(MMath::rotate(180.0f, Vec3(0.0f, 1.0f, 0.0f))), Vec3(0.25f, 0.25f, 0.25f));
		}
		else {
			knight->AddComponent(blackChess);
			knight->AddComponent<TransformComponent>(nullptr, Vec3((-4.4f + 1.25f) + (i - 2) * 6.25f, -4.4f + 8.75f, 0.0f), QMath::toQuaternion(MMath::rotate(90.0f, Vec3(1.0f, 0.0f, 0.0f))), Vec3(0.25f, 0.25f, 0.25f));
		}

		knight->OnCreate();
		Actors.insert(std::pair(knightN, knight));
	}

	for (int i = 0; i < 4; i++) {
		std::string bishopN = "bishopN" + std::to_string(i);
		Ref<Actor> bishop = std::make_shared<Actor>(board.get());
		bishop->AddComponent(phongShader);
		bishop->AddComponent(bishopMesh);

		if (i < 2) {
			bishop->AddComponent(whiteChess);
			bishop->AddComponent<TransformComponent>(nullptr, Vec3((-4.4f + 2.5f) + i * 3.75f, -4.4f, 0.0f), QMath::toQuaternion(MMath::rotate(90.0f, Vec3(1.0f, 0.0f, 0.0f))) * QMath::toQuaternion(MMath::rotate(180.0f, Vec3(0.0f, 1.0f, 0.0f))), Vec3(0.25f, 0.25f, 0.25f));
		}
		else {
			bishop->AddComponent(blackChess);
			bishop->AddComponent<TransformComponent>(nullptr, Vec3((-4.4f + 2.5f) + (i - 2) * 3.75f, -4.4f + 8.75f, 0.0f), QMath::toQuaternion(MMath::rotate(90.0f, Vec3(1.0f, 0.0f, 0.0f))), Vec3(0.25f, 0.25f, 0.25f));
		}

		bishop->OnCreate();
		Actors.insert(std::pair(bishopN, bishop));
	}

	for (int i = 0; i < 2; i++) {
		std::string kingN = "kingN" + std::to_string(i);
		Ref<Actor> king = std::make_shared<Actor>(board.get());
		king->AddComponent(phongShader);
		king->AddComponent(kingMesh);

		if (i < 1) {
			king->AddComponent(whiteChess);
			king->AddComponent<TransformComponent>(nullptr, Vec3((-4.4f + 3.75f) + i * 1.25f, -4.4f, 0.0f), QMath::toQuaternion(MMath::rotate(90.0f, Vec3(1.0f, 0.0f, 0.0f))) * QMath::toQuaternion(MMath::rotate(180.0f, Vec3(0.0f, 1.0f, 0.0f))), Vec3(0.2f, 0.2f, 0.35f));
		}
		else {
			king->AddComponent(blackChess);
			king->AddComponent<TransformComponent>(nullptr, Vec3((-4.4f + 3.75f) + (i - 1) * 1.25f, -4.4f + 8.75f, 0.0f), QMath::toQuaternion(MMath::rotate(90.0f, Vec3(1.0f, 0.0f, 0.0f))), Vec3(0.2f, 0.2f, 0.35f));
		}

		king->OnCreate();
		Actors.insert(std::pair(kingN, king));
	}

	for (int i = 0; i < 2; i++) {
		std::string queenN = "queenN" + std::to_string(i);

		Ref<Actor> queen = std::make_shared<Actor>(board.get());
		queen->AddComponent(phongShader);
		queen->AddComponent(queenMesh);

		if (i < 1) {
			queen->AddComponent(whiteChess);
			queen->AddComponent<TransformComponent>(nullptr, Vec3((-4.4f + 5.0f) + i * 1.25f, -4.4f, 0.0f), QMath::toQuaternion(MMath::rotate(90.0f, Vec3(1.0f, 0.0f, 0.0f))) * QMath::toQuaternion(MMath::rotate(180.0f, Vec3(0.0f, 1.0f, 0.0f))), Vec3(0.2f, 0.2f, 0.3f));
		}
		else {
			queen->AddComponent(blackChess);
			queen->AddComponent<TransformComponent>(nullptr, Vec3((-4.4f+ 5.0f) + (i - 1) * 1.25f, -4.4f + 8.75f, 0.0f), QMath::toQuaternion(MMath::rotate(90.0f, Vec3(1.0f, 0.0f, 0.0f))), Vec3(0.2f, 0.2f, 0.3f));
		}

		queen->OnCreate();
		Actors.insert(std::pair(queenN, queen));
	}

	return true;
}

void Scene1g::OnDestroy() {
	Debug::Info("Deleting assets Scene1g: ", __FILE__, __LINE__);

	
	// clear the map
	Actors.clear();

	
}

void Scene1g::HandleEvents(const SDL_Event& sdlEvent) {
	switch (sdlEvent.type) {
	case SDL_KEYDOWN:
		switch (sdlEvent.key.keysym.scancode) {
		case SDL_SCANCODE_T:
			drawInWireMode = !drawInWireMode;
			break;

		case SDL_SCANCODE_E:
			Actors.at("TestN0")->GetComponent<TransformComponent>()->SetPos(-4.4f, -1.5f, 0.0f);
			break;
			
			
		}
		break;

	case SDL_MOUSEMOTION:
		break;

	case SDL_MOUSEBUTTONDOWN:
		break;

	case SDL_MOUSEBUTTONUP:
		break;

	default:
		break;
	}
}

void Scene1g::Update(const float deltaTime) {

	static float angle = 0.0f;
	angle += 20.0f * deltaTime;

	
	// makes board spin
	Matrix4 boardModel = MMath::rotate(angle, Vec3(0.0f, 0.0f, 1.0f)) *
		MMath::rotate(-90.0f, Vec3(0.0f, 1.0f, 0.0f)) *
		MMath::rotate(90.0f, Vec3(0.0f, 1.0f, 0.0f));

	board->GetComponent<TransformComponent>()->SetOrientation(QMath::toQuaternion(boardModel));

	Actors.at("TestN0")->GetComponent<PhysicsComponent>()->UpdateP(deltaTime, Actors.at("TestN0"));
	Actors.at("TestN1")->GetComponent<PhysicsComponent>()->UpdateP(deltaTime, Actors.at("TestN1"));

	collisionSystem.Update(deltaTime);
}

void Scene1g::Render() const {
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


	glUseProgram(phongShader->GetProgram());
	glUniformMatrix4fv(phongShader->GetUniformID("projectionMatrix"), 1, GL_FALSE, camera->GetProjectionMatrix());
	glUniformMatrix4fv(phongShader->GetUniformID("viewMatrix"), 1, GL_FALSE, camera->GetViewMatrix());


	// get a reference instead of creating a copy 
	for (auto& actor : Actors) {
		// get the string part (first value) of the specific unordered map pair
		RenderMap(actor.first);
	}

	glUniform3fv(phongShader->GetUniformID("lightPos"), 1, lightPos);

	glUseProgram(0);
}

void Scene1g::RenderMap(const std::string& actorName) const
{
	// create an iterator 
	auto it = Actors.find(actorName);
	// loop through the entire iterator (all the actors in the unordered map) and render them
	if (it != Actors.end()) {
		// temp actor that accesses the "second value" (the actor) of the speific unordered map pair 
		// also dereferences the iterator so it keeps looping
		Ref<Actor> actor = it->second;
		glUniformMatrix4fv(actor->GetComponent<ShaderComponent>()->GetUniformID("modelMatrix"), 1, GL_FALSE, actor->GetModelMatrix());
		glBindTexture(GL_TEXTURE_2D, actor->GetComponent<MaterialComponent>()->getTextureID());
		actor->GetComponent<MeshComponent>()->Render(GL_TRIANGLES);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}


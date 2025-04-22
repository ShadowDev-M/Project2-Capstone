#include <glew.h>
#include <iostream>
#include <SDL.h>
#include "Scene0g.h"
#include <MMath.h>
#include "Debug.h"
#include "MeshComponent.h"
#include "ShaderComponent.h"
#include "TransformComponent.h"
#include "MaterialComponent.h"

Scene0g::Scene0g() :mario{nullptr}, drawInWireMode{false} {
	Debug::Info("Created Scene0: ", __FILE__, __LINE__);
}

Scene0g::~Scene0g() {
	Debug::Info("Deleted Scene0: ", __FILE__, __LINE__);
}

bool Scene0g::OnCreate() {
	Debug::Info("Loading assets Scene0: ", __FILE__, __LINE__);
	
	//
	camera = new CameraActor(nullptr, 45.0f, (16.0f / 9.0f), 0.5f, 100.0f);
	camera->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, -5.0f), Quaternion(), Vec3(1.0f, 1.0f, 1.0f));

	camera->OnCreate();

	mario = new Actor(nullptr);
	mario->AddComponent<MeshComponent>(nullptr, "meshes/Mario.obj");
	mario->AddComponent<ShaderComponent>(nullptr, "shaders/texturePhongVert.glsl", "shaders/texturePhongFrag.glsl");
	mario->AddComponent<TransformComponent>(nullptr, Vec3(), Quaternion(0.0f, Vec3(0.0f, 1.0f, 0.0f)), Vec3(1.0f, 1.0f, 1.0f));
	mario->AddComponent<MaterialComponent>(nullptr, "textures/mario_fire.png");

	mario->OnCreate();
	
	hammer = new Actor(mario);
	hammer->AddComponent<MeshComponent>(nullptr, "meshes/Hammer.obj");
	hammer->AddComponent<ShaderComponent>(nullptr, "shaders/texturePhongVert.glsl", "shaders/texturePhongFrag.glsl");
	hammer->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, 0.0f), Quaternion(0.0f, Vec3(0.0f, 1.0f, 0.0f)), Vec3(1.0f, 1.0f, 1.0f));
	hammer->AddComponent<MaterialComponent>(nullptr, "textures/hammer_BaseColor.png");

	hammer->OnCreate();

	//
	mario->ListComponents();

	return true;
}

void Scene0g::OnDestroy() {
	Debug::Info("Deleting assets Scene0: ", __FILE__, __LINE__);
	
	mario->OnDestroy();
	delete mario;

	hammer->OnDestroy();
	delete hammer;
	
}

void Scene0g::HandleEvents(const SDL_Event &sdlEvent) {
	switch( sdlEvent.type ) {
    case SDL_KEYDOWN:
		switch (sdlEvent.key.keysym.scancode) {
			case SDL_SCANCODE_W:
				drawInWireMode = !drawInWireMode;
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

void Scene0g::Update(const float deltaTime) {
	
	static float angle = 0.0f;
	angle += 20.0f * deltaTime;	
	
	// Switch to transform component

	Matrix4 marioModel	= MMath::rotate(angle, Vec3(0.0f, 1.0f, 0.0f)) *
		MMath::rotate(-90.0f, Vec3(1.0f, 0.0f, 0.0f)) *
		MMath::rotate(90.0f, Vec3(1.0f, 0.0f, 0.0f));


	Matrix4 hammerModel = MMath::rotate(-90.0f, Vec3(1.0f, 0.0f, 0.0f));

	// Change to work with transform component

	// Using transform components settransform, converting Matrix4 to Quaternion
	// Could probably just create a seperate function in the actual components code to simplify this

	// Pos, Ori, Scale
	mario->GetComponent<TransformComponent>()->SetTransform(Vec3(), QMath::toQuaternion(marioModel), Vec3(1.0f, 1.0f, 1.0f));

	hammer->GetComponent<TransformComponent>()->SetTransform(Vec3(1.1f, 0.1f, 0.5f), QMath::toQuaternion(hammerModel), Vec3(1.0f, 1.0f, 1.0f));

}

void Scene0g::Render() const {
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

	Ref<ShaderComponent> shader = mario->GetComponent<ShaderComponent>();
	
	
	
	glUseProgram(shader->GetProgram());
	glUniformMatrix4fv(shader->GetUniformID("projectionMatrix"), 1, GL_FALSE, camera->GetProjectionMatrix());
	glUniformMatrix4fv(shader->GetUniformID("viewMatrix"), 1, GL_FALSE, camera->GetViewMatrix());
	
	// Model Matrix
	glUniformMatrix4fv(shader->GetUniformID("modelMatrix"), 1, GL_FALSE, mario->GetComponent<TransformComponent>()->GetTransformMatrix());
	glBindTexture(GL_TEXTURE_2D, mario->GetComponent<MaterialComponent>()->getTextureID());
	mario->GetComponent<MeshComponent>()->Render(GL_TRIANGLES);
	glBindTexture(GL_TEXTURE_2D, 0);	


	// Model Matrix
	glUniformMatrix4fv(shader->GetUniformID("modelMatrix"), 1, GL_FALSE, hammer->GetModelMatrix());
	glBindTexture(GL_TEXTURE_2D, hammer->GetComponent<MaterialComponent>()->getTextureID());
	hammer->GetComponent<MeshComponent>()->Render(GL_TRIANGLES);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glUseProgram(0);
}



	

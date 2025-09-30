#include "SceneGraph.h"
#include "ExampleXML.h"
#include "InputManager.h"


void SceneGraph::setUsedCamera(Ref<CameraComponent> newCam) {
	usedCamera = newCam;

	//If camera component is non existent, or if intentionally left blank, try to get the next random camera
	if (!newCam) {
		//Set camera to first camera found in loop so it doesn't crash
		for (auto& pair : Actors) {
			Ref<CameraComponent> cam = pair.second->GetComponent<CameraComponent>();
			if (cam) {
				usedCamera = cam;
				return;
			}
		}
		//If it doesn't return, then it will probably crash 
		//TODO: handle camera if there ends up no valid camera to use

		
	}
}


void SceneGraph::LoadActor(const char* name_, Ref<Actor> parent) {
	
	Ref<Actor> actor_ = std::make_shared<Actor>(parent.get(), name_);

	// if statements to check whether or not a specific component exists
	// added this because before the engine would crash because it would be trying to add an actor that didn't have a mesh/material/shader
	if (XMLObjectFile::hasComponent<MaterialComponent>(name_)) {
		std::string materialName = XMLObjectFile::getComponent<MaterialComponent>(name_);
		if (!materialName.empty()) {
			Ref<MaterialComponent> materialComponent = AssetManager::getInstance().GetAsset<MaterialComponent>(materialName);
			if (materialComponent) {
				actor_->AddComponent<MaterialComponent>(materialComponent);
			}
		}
	}

	if (XMLObjectFile::hasComponent<ShaderComponent>(name_)) {
		std::string shaderName = XMLObjectFile::getComponent<ShaderComponent>(name_);
		if (!shaderName.empty()) {
			Ref<ShaderComponent> shaderComponent = AssetManager::getInstance().GetAsset<ShaderComponent>(shaderName);
			if (shaderComponent) {
				actor_->AddComponent<ShaderComponent>(shaderComponent);
			}
		}
	}

	if (XMLObjectFile::hasComponent<MeshComponent>(name_)) {
		std::string meshName = XMLObjectFile::getComponent<MeshComponent>(name_);
		if (!meshName.empty()) {
			Ref<MeshComponent> meshComponent = AssetManager::getInstance().GetAsset<MeshComponent>(meshName);
			if (meshComponent) {
				actor_->AddComponent<MeshComponent>(meshComponent);
			}
		}
	}

	
	
	actor_->AddComponent<TransformComponent>(std::apply([](auto&&... args) {
		return new TransformComponent(args...);
		}, XMLObjectFile::getComponent<TransformComponent>(name_)));
	

	if (XMLObjectFile::hasComponent<CameraComponent>(name_)) {
		std::cout << "Has a Camera" << std::endl; 
		actor_->AddComponent<CameraComponent>(actor_, 45.0f, (16.0f / 9.0f), 0.5f, 100.0f);
	}

	actor_->OnCreate();
	AddActor(actor_);


}

Ref<Actor> SceneGraph::pickColour(int mouseX, int mouseY) {


	//Width of SDL Window
	int w, h;
	SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &w, &h);

	float aspectRatio = static_cast<float>(w) / static_cast<float>(h);

	//size of the imgui window used for docking
	Vec2 windowSize = Vec2(InputManager::getInstance().getMouseMap()->dockingSize.x, InputManager::getInstance().getMouseMap()->dockingSize.y);


	//pos of top left corner of docking window
	GLint xPos = InputManager::getInstance().getMouseMap()->dockingPos.x;
	GLint yPos = InputManager::getInstance().getMouseMap()->dockingPos.y;

	GLsizei xSize;
	GLsizei ySize;




	// Calculate scaled dimensions based on aspect ratio
	if (windowSize.x / aspectRatio <= windowSize.y)
	{
		xSize = windowSize.x;
		ySize = windowSize.x / aspectRatio;
	}
	else
	{
		ySize = windowSize.y;
		xSize = windowSize.y * aspectRatio;
	}


	//Add to the position to move to where the image is centred (non used space in the window would be counted otherwise)
	ImVec2 imagePos = ImVec2((windowSize.x - xSize) * 0.5f, (windowSize.y - ySize) * 0.5f);
	xPos += imagePos.x;
	yPos += imagePos.y;

	//proper height 
	GLint glY = h - (yPos + ySize);


	//use the picking buffer so it is seperated
	glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO);
	glViewport(xPos, glY, xSize, ySize);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	//get the special shader for picking and set its uniforms
	glUseProgram(pickerShader->GetProgram());
	glUniformMatrix4fv(pickerShader->GetUniformID("uProjection"), 1, GL_FALSE, getUsedCamera()->GetProjectionMatrix());
	glUniformMatrix4fv(pickerShader->GetUniformID("uView"), 1, GL_FALSE, getUsedCamera()->GetViewMatrix());

	for (auto& actor : Actors) {

		if (!actor.second->GetComponent<MeshComponent>()) { continue; }//no meshcomponent for actor

		//set matrix with its camera (i forgot to put in camera parametre and spent 5 hours why it was coming back as completely black)
		glUniformMatrix4fv(pickerShader->GetUniformID("uModel"), 1, GL_FALSE, actor.second->GetModelMatrix(getUsedCamera()));

		//encode the id of the actor as rgb
		Vec3 idColor = Actor::encodeID(actor.second->getId());

		//send over the rbg to the shader to use as its rendered colour
		glUniform3fv(pickerShader->GetUniformID("uIDColor"), 1, &idColor.x);

		glBindTexture(GL_TEXTURE_2D, 0);
		actor.second->GetComponent<MeshComponent>()->Render();
	}

	//rgb pixel data
	unsigned char pixel[3];

	glReadBuffer(GL_COLOR_ATTACHMENT0);

	//get the mouse click's rgb pixel data
	glReadPixels(mouseX, h - mouseY, 1, 1,
		GL_RGB, GL_UNSIGNED_BYTE, pixel);

	//reverse selected pixel's rgb and decode into an id
	uint32_t pickedID = Actor::decodeID(pixel[0], pixel[1], pixel[2]);

	//unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);

	//check if the clicked pixel's colour id is the same as any of the actors
	for (auto& actor : Actors) {
		if (actor.second->getId() == pickedID) return actor.second;
	}

	return nullptr; // nothing clicked
}

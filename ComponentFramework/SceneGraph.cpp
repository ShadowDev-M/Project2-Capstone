#include "SceneGraph.h"
#include "ExampleXML.h"

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
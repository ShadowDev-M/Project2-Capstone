#include "SceneGraph.h"
#include "ExampleXML.h"

void SceneGraph::LoadActor(const char* name_, Ref<Actor> parent) {
	
	

	Ref<Actor> actor_ = std::make_shared<Actor>(parent.get(), name_);

	actor_->AddComponent<MaterialComponent>(AssetManager::getInstance().GetAsset<MaterialComponent>(XMLObjectFile::getComponent<MaterialComponent>(name_)));
	actor_->AddComponent<ShaderComponent>(AssetManager::getInstance().GetAsset<ShaderComponent>(XMLObjectFile::getComponent<ShaderComponent>(name_)));
	actor_->AddComponent<MeshComponent>(AssetManager::getInstance().GetAsset<MeshComponent>(XMLObjectFile::getComponent<MeshComponent>(name_)));
	actor_->AddComponent<TransformComponent>(std::apply([](auto&&... args) {
		return new TransformComponent(args...);
		}, XMLObjectFile::getComponent<TransformComponent>(name_)));



	actor_->OnCreate();
	AddActor(actor_);


}
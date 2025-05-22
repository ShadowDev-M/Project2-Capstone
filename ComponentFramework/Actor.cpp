#include "Actor.h"
#include "Debug.h"
#include "TransformComponent.h"
#include "CameraActor.h"

Actor::Actor(Component* parent_):Component(parent_) {}

Actor::Actor(Component* parent_, const std::string& actorName_): Component(parent_), actorName(actorName_) {}


bool Actor::OnCreate() {
	if (isCreated) return true;
	Debug::Info("Loading assets for Actor: ", __FILE__, __LINE__);
	
	// Loops over all components
	for (auto &component : components) {
		if (component->OnCreate() == false) {
			Debug::Error("Loading assets for Actor/Components: ", __FILE__, __LINE__);
			isCreated = false;
			return isCreated;
		}
	}
	isCreated = true;
	return isCreated;
}

Actor::~Actor() {
	OnDestroy();
}

void Actor::OnDestroy() {
	Debug::Info("Deleting assets for Actor: ", __FILE__, __LINE__);
	RemoveAllComponents();
	isCreated = false;
}



void Actor::Update(const float deltaTime) {
	
}

void Actor::Render()const {}

void Actor::RemoveAllComponents() {
	components.clear();
}


//
void Actor::ListComponents() const {
	std::cout << typeid(*this).name() << " contains the following components:\n";
	for (auto &component : components) {
		std::cout << typeid(*component).name() << std::endl;
	}
	std::cout << '\n';
}


Matrix4 Actor::GetModelMatrix(Ref<Actor> camera_) {
	Ref<CameraActor> camera = std::dynamic_pointer_cast<CameraActor>(camera_);

	Ref<TransformComponent> transform = GetComponent<TransformComponent>();
	
	

	if (transform) {
		modelMatrix = transform->GetTransformMatrix();
	}
	else {
		modelMatrix.loadIdentity();
	}
	if (parent) { 
		modelMatrix = dynamic_cast<Actor*>(parent)->GetModelMatrix(camera) * modelMatrix;
	}
	else {
		modelMatrix = MMath::translate(-camera->GetComponent<TransformComponent>()->GetPosition()) * modelMatrix;
	}
	return modelMatrix;
}

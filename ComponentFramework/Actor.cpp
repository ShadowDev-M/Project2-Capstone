#include "pch.h"
#include "Actor.h"
#include "TransformComponent.h"
#include "LightComponent.h"
#include "MeshComponent.h"
#include "InputManager.h"
#include "SceneGraph.h"
#include "AnimatorComponent.h"
#include "AnimationSystem.h"
static uint32_t idCounter = 1;

Actor::Actor(Component* parent_) :Component(parent_) { id = idCounter++; }

Actor::Actor(Component* parent_, const std::string& actorName_): Component(parent_), actorName(actorName_) { id = idCounter++; }


bool Actor::OnCreate() {
	if (isCreated) return true;
	Debug::Info("Loading assets for Actor: ", __FILE__, __LINE__);
	
	// Loops over all components
	for (auto& component : components) {

		std::string componentType = static_cast<std::string>(typeid(component.get()).name()).substr(6); //TODO: substr removes the 'class ' that gets added when getting the name from a typeid, 

		if (std::dynamic_pointer_cast<ShaderComponent>(component) ||
			std::dynamic_pointer_cast<MeshComponent>(component) ||
			std::dynamic_pointer_cast<MaterialComponent>(component) ||
			std::dynamic_pointer_cast<ScriptComponent>(component)) {
			continue;
		}

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
	if (!isCreated) return;

	Debug::Info("Deleting assets for Actor: ", __FILE__, __LINE__);
	RemoveAllComponents();
	isCreated = false;

	
}

void Actor::Update(const float deltaTime) {}
void Actor::Render()const {}

void Actor::RemoveAllComponents() {
	components.clear();
}

void Actor::PushToAnimationSystem(Ref<Component> component)
{
	Ref<MeshComponent> mesh = std::dynamic_pointer_cast<MeshComponent>(component);
	Ref<Animation> animation = std::dynamic_pointer_cast<Animation>(component);

	if (mesh) {
		AnimationSystem::getInstance().PushMeshToWorker(mesh);
	}
	else if (animation) {
		AnimationSystem::getInstance().PushAnimationToWorker(animation);
	}
}

void Actor::ListComponents() const {
	std::cout << typeid(*this).name() << " contains the following components:\n";
	for (auto &component : components) {
		std::cout << typeid(*component).name() << std::endl;
	}
	std::cout << '\n';
}

Matrix4 Actor::GetModelMatrix() {
	Ref<TransformComponent> transform = GetComponent<TransformComponent>();
	modelMatrix = transform ? transform->GetTransformMatrix() : Matrix4();

	// multiples the parents model matrix by the childs (hierarchly/recursivly multiples parent matrix/transform)
	// TODO: find something better for parent/child movement
	if (parent && dynamic_cast<Actor*>(parent)) {
		modelMatrix = dynamic_cast<Actor*>(parent)->GetModelMatrix() * modelMatrix;
	}

	return modelMatrix;
}
	
#ifdef _DEBUG
#define DEBUG_NEW new(__FILE__, __LINE__)
#define new DEBUG_NEW
#endif

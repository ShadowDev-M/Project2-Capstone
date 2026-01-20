#include "pch.h"
#include "PhysicsComponent.h"
#include "TransformComponent.h"

PhysicsComponent::PhysicsComponent(Component* parent_, float mass_) : Component(parent), vel(), accel(), mass(mass_) {}

PhysicsComponent::~PhysicsComponent()
{
}

void PhysicsComponent::Update(float deltaTime)
{

}

void PhysicsComponent::ApplyForce(Vec3 force)
{
	accel = force / mass;
}

void PhysicsComponent::UpdateP(float deltaTime, Ref<Actor> actor)
{
	// get the transform component of the actor
	Ref<TransformComponent> ts = actor->GetComponent<TransformComponent>();

	// do what the update did from body but use position from the transform component
	Vec3 tsPos = ts->GetPosition();
	tsPos += vel * deltaTime + 0.5f * accel * deltaTime * deltaTime;
	ts->SetPos(tsPos.x, tsPos.y, tsPos.z);

	vel += accel * deltaTime;
}

bool PhysicsComponent::OnCreate()
{
	//if (actor->GetComponent<TransformComponent>().get() == nullptr) {
	//	Debug::Error("The Actor must have a TransformComponent - ignored ", __FILE__, __LINE__);
	//	return false;
	//}
	if (isCreated == true) return true;
	
	return false;
}

void PhysicsComponent::OnDestroy()
{
}

void PhysicsComponent::Render() const
{
}


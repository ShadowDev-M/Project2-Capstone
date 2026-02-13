#include "pch.h"
#include "PhysicsComponent.h"

PhysicsComponent::PhysicsComponent(Component* parent_, PhysicsState state_, float mass_, bool useGrav_, float drag_, float angularDrag_, float friction_, float restitution_) :
	Component(parent_), state(state_), mass(mass_), useGravity(useGrav_), drag(drag_), angularDrag(angularDrag_), friction(friction_), restitution(restitution_)
{

	vel = Vec3(0.0f, 0.0f, 0.0f);
	accel = Vec3(0.0f, 0.0f, 0.0f);
	angularVel = Vec3(0.0f, 0.0f, 0.0f);
	angularAcc = Vec3(0.0f, 0.0f, 0.0f);
}
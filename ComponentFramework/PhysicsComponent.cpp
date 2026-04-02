#include "pch.h"
#include "PhysicsComponent.h"
#include "CollisionComponent.h"
PhysicsComponent::PhysicsComponent(Component* parent_, PhysicsState state_, PhysicsConstraints constraints_, float mass_, bool useGrav_, float drag_, float angularDrag_, float friction_, float restitution_) :
	Component(parent_), state(state_), constraints(constraints_), mass(mass_), useGravity(useGrav_), drag(drag_), angularDrag(angularDrag_), friction(friction_), restitution(restitution_)
{

	vel = Vec3(0.0f, 0.0f, 0.0f);
	accel = Vec3(0.0f, 0.0f, 0.0f);
	angularVel = Vec3(0.0f, 0.0f, 0.0f);
	angularAcc = Vec3(0.0f, 0.0f, 0.0f);
}



Vec3 PhysicsComponent::inertiaTensorBody(Ref<Actor> user, Vec3& torqueAccumulator) {
    Ref<CollisionComponent> collider = user->GetComponent<CollisionComponent>();
    Ref<TransformComponent> transform = user->GetComponent<TransformComponent>();
    if (collider->getType() != ColliderType::OBB) return Vec3();

    Vec3 h = collider->getWorldHalfExtents(transform) * 2;
    float m = getInverseMass();

    float inv_Ixx = 12.f / (m * (h.y * h.y + h.z * h.z));
    float inv_Iyy = 12.f / (m * (h.x * h.x + h.z * h.z));
    float inv_Izz = 12.f / (m * (h.x * h.x + h.y * h.y));

    return Vec3(
        inv_Ixx * torqueAccumulator.x,
        inv_Iyy * torqueAccumulator.y,
        inv_Izz * torqueAccumulator.z
    );
}

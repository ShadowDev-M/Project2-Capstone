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



void PhysicsComponent::inertiaTensorBody(Ref<Actor> user) {
    Ref<CollisionComponent> collider = user->GetComponent<CollisionComponent>();
    Ref<TransformComponent> transform = user->GetComponent<TransformComponent>();
    if (collider->getType() != ColliderType::OBB) return;

    Vec3 h = collider->getWorldHalfExtents(transform) * 2;
    float m = getInverseMass();

    Matrix3 invI_body;
    invI_body.setColumn(Matrix3::Column::zero, Vec3(12.f / (m * (h.y * h.y + h.z * h.z)), 0.f, 0.f));
    invI_body.setColumn(Matrix3::Column::one, Vec3(0.f, 12.f / (m * (h.x * h.x + h.z * h.z)), 0.f));
    invI_body.setColumn(Matrix3::Column::two, Vec3(0.f, 0.f, 12.f / (m * (h.x * h.x + h.y * h.y))));



    Matrix3 R = MMath::toMatrix3(transform->GetOrientation());
    rotationalInertia = invI_body * MMath::transpose(R);

    inverseRotationalInertia = MMath::inverse(rotationalInertia);

}

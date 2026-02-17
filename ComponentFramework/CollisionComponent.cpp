#include "pch.h"
#include "CollisionComponent.h"
#include "Actor.h"
Vec3 CollisionComponent::getWorldCentre(Ref<TransformComponent> transform_) const
{
	return Vec3(transform_->getParent()->GetModelMatrix() * Vec4(centre, 1.0f));
}
Vec3 CollisionComponent::getWorldCentrePosA(Ref<TransformComponent> transform_) const
{
	return Vec3(transform_->getParent()->GetModelMatrix() * Vec4(centrePosA, 1.0f));
}
Vec3 CollisionComponent::getWorldCentrePosB(Ref<TransformComponent> transform_) const
{
	return Vec3(transform_->getParent()->GetModelMatrix() * Vec4(centrePosB, 1.0f));
}

// this default constructor has all data for all differnet collider types
CollisionComponent::CollisionComponent(Component* parent_, ColliderState state_, ColliderType type_, bool isTrigger_, float radius_, Vec3 centre_, Vec3 centrePosA_, Vec3 centrePosB_, Vec3 halfExtents_, Quaternion orientation_) :
	Component(parent_), colliderState(state_), colliderType(type_), isTrigger(isTrigger_), radius(radius_), centre(centre_), centrePosA(centrePosA_), centrePosB(centrePosB_), halfExtents(halfExtents_), orientation(orientation_)
{
}

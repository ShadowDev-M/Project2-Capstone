#include "pch.h"
#include "CollisionComponent.h"
#include "Actor.h"
Vec3 CollisionComponent::getWorldScale(Ref<TransformComponent> transform_) const
{
	// manually extracting the scale from the parents model matrix
	Matrix4 M = transform_->getParent()->GetModelMatrix();
	Vec3 col0 = Vec3(M * Vec4(1.0f, 0.0f, 0.0f, 0.0f));
	Vec3 col1 = Vec3(M * Vec4(0.0f, 1.0f, 0.0f, 0.0f));
	Vec3 col2 = Vec3(M * Vec4(0.0f, 0.0f, 1.0f, 0.0f));
	return Vec3(VMath::mag(col0), VMath::mag(col1), VMath::mag(col2));
}
Vec3 CollisionComponent::getWorldCentre(Ref<TransformComponent> transform_) const
{
	return Vec3(transform_->getParent()->GetModelMatrix() * Vec4(centre, 1.0f));
}
float CollisionComponent::getWorldRadius(Ref<TransformComponent> transform_) const
{
	Vec3 scale = getWorldScale(transform_);
	float maxScale = std::max(std::max(scale.x, scale.y), scale.z);
	return radius * maxScale;
}
float CollisionComponent::getWorldCapsuleRadius(Ref<TransformComponent> transform_) const
{
	Vec3 scale = getWorldScale(transform_);
	float maxScale = std::max(scale.x, scale.z);
	return radius * maxScale;

}
Vec3 CollisionComponent::getWorldCentrePosA(Ref<TransformComponent> transform_) const
{
	return Vec3(transform_->getParent()->GetModelMatrix() * Vec4(centrePosA, 1.0f));
}
Vec3 CollisionComponent::getWorldCentrePosB(Ref<TransformComponent> transform_) const
{
	return Vec3(transform_->getParent()->GetModelMatrix() * Vec4(centrePosB, 1.0f));
}

Vec3 CollisionComponent::getWorldHalfExtents(Ref<TransformComponent> transform_) const
{
	Vec3 scale = getWorldScale(transform_);
	return Vec3(halfExtents.x * scale.x, halfExtents.y * scale.y, halfExtents.z * scale.z);
}

void CollisionComponent::getWorldAxes(Ref<TransformComponent> transform_, Vec3 axes[3]) const
{
	Matrix4 M = transform_->getParent()->GetModelMatrix();
	
	// rotating the orienation around x,y,z axis in order to get local coords
	Vec3 localX = orientation * Vec3(1.0f, 0.0f, 0.0f);
	Vec3 localY = orientation * Vec3(0.0f, 1.0f, 0.0f);
	Vec3 localZ = orientation * Vec3(0.0f, 0.0f, 1.0f);

	axes[0] = VMath::normalize(Vec3(M * Vec4(localX.x, localX.y, localX.z, 0.0f)));
	axes[1] = VMath::normalize(Vec3(M * Vec4(localY.x, localY.y, localY.z, 0.0f)));
	axes[2] = VMath::normalize(Vec3(M * Vec4(localZ.x, localZ.y, localZ.z, 0.0f)));
}

// this default constructor has all data for all differnet collider types
CollisionComponent::CollisionComponent(Component* parent_, ColliderState state_, ColliderType type_, bool isTrigger_, float radius_, Vec3 centre_, Vec3 centrePosA_, Vec3 centrePosB_, Vec3 halfExtents_, Quaternion orientation_) :
	Component(parent_), colliderState(state_), colliderType(type_), isTrigger(isTrigger_), radius(radius_), centre(centre_), centrePosA(centrePosA_), centrePosB(centrePosB_), halfExtents(halfExtents_), orientation(orientation_)
{
}

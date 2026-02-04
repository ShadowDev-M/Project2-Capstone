#include "pch.h"
#include "CollisionComponent.h"

// this default constructor has all data for all differnet collider types
// at first I wasn't going to do this 
CollisionComponent::CollisionComponent(Component* parent_, ColliderType type_, float radius_, Vec3 centre_, Vec3 centrePosA_, Vec3 centrePosB_, Vec3 halfExtents_, Quaternion orientation_) :
	Component(parent_), colliderType(type_), radius(radius_), centre(centre_), centrePosA(centrePosA_), centrePosB(centrePosB_), halfExtents(halfExtents_), orientation(orientation_)
{
}

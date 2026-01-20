#include "pch.h"
#include "CollisionComponent.h"

// if this constructor is used collider type is sphere
CollisionComponent::CollisionComponent(Component* parent_, float radius_) : 
	Component(parent_),
	radius(radius_),
	colliderType(ColliderType::SPHERE)
{}

// if this constructor is used collider type is AABB
CollisionComponent::CollisionComponent(Component* parent_, Vec3 halfExtents_) : 
	Component(parent_),
	halfExtents(halfExtents_),
	colliderType(ColliderType::AABB)
{}

// if this constructor is used collider type is plane
CollisionComponent::CollisionComponent(Component* parent_, Plane plane_) : 
	Component(parent_),
	plane(plane_),
	colliderType(ColliderType::PLANE)
{}
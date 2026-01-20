#pragma once
#include "TransformComponent.h"
using namespace MATHEX;

enum class ColliderType {
	SPHERE,
	AABB,
	PLANE,
};

/// For an Axis Aligned Bounding Box, their are many ways you could define the box.
/// My favorite way, the easiest to understand way is pick the center location of 
/// the box, then specify the radius from that center in 
/// the x, y,and z dimensions. Umer calls the radius the halfExtent
struct AABB {
	Vec3 center;
	Vec3 halfExtents;
};

class CollisionComponent : public Component {
	friend class CollisionSystem;
	CollisionComponent(const CollisionComponent&) = delete;
	CollisionComponent(CollisionComponent&&) = delete;
	CollisionComponent& operator = (const CollisionComponent&) = delete;
	CollisionComponent& operator = (CollisionComponent&&) = delete;
protected:
	ColliderType colliderType;
	float radius; /// Sphere collision
	Vec3 halfExtents; /// AABB
	Plane plane; /// Plane 

public:
	CollisionComponent(Component* parent_, float radius_);
	CollisionComponent(Component* parent_, Vec3 halfExtents_);
	CollisionComponent(Component* parent_, Plane plane_);
	bool OnCreate() { return true; }
	void OnDestroy() {}
	void Update(const float deltaTime_) {}
	void Render()const {}
};

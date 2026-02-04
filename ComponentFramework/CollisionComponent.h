#pragma once
#include "TransformComponent.h"

using namespace MATH;

// There are two ways to set all this up, I'm going with option 2
// 1. I create indiviual collision components for each different collider type
// 2. I create one collision component that holds all data for all different collider types,
// then in engine, when adding a component, from the inspector window the user can decide which
// collider they want for the specific object, and it'll show the specific variables for that
// The benefits of option 2 is that it simplifies alot of things, i.e XML component save & load

// Different collider types
// TODO: Inspector window, switch different types
enum class ColliderType {
	Sphere,
	Cylinder,
	Capsule,
	AABB,
	OBB
};

// Creating different structs for the different collider types
// the structs will mostly be used in the collision system to help out 
// with collision detection of different collider types

// MATHEX library has a sphere.h, but I want to keep everything inline so creating my own struct
struct Sphere {
	Vec3 centre;
	float radius;
};

struct Cylinder {
	float radius;
	Vec3 capCentrePosA;
	Vec3 capCentrePosB;
};

struct Capsule {
	float radius;
	Vec3 sphereCentrePosA;
	Vec3 sphereCentrePosB;
};

struct AABB {
	Vec3 centre;
	Vec3 halfExtents;
};

struct OBB {
	Vec3 centre;
	Vec3 halfExtents;
	Quaternion orientation;
};

class CollisionComponent : public Component {
	friend class CollisionSystem;
	CollisionComponent(const CollisionComponent&) = delete;
	CollisionComponent(CollisionComponent&&) = delete;
	CollisionComponent& operator = (const CollisionComponent&) = delete;
	CollisionComponent& operator = (CollisionComponent&&) = delete;
protected:
	ColliderType colliderType; 
	
	bool isTrigger; // determines the type of response 

	float radius; /// Sphere, Cylinder, Capsule
	Vec3 centre; // Sphere, AABB, OBB
	Vec3 centrePosA; // Cylinder, Capsule
	Vec3 centrePosB; // Cylinder, Capsule
	Vec3 halfExtents; /// AABB, OBB
	Quaternion orientation; // OBB 

	// should centre and orientation be tranform position and orientation? search it up

public:
	CollisionComponent(Component* parent_ = nullptr, ColliderType type_ = ColliderType::Sphere, float radius_ = 0.0f, Vec3 centre_ = Vec3(0.0f, 0.0f, 0.0f),
		Vec3 centrePosA_ = Vec3(0.0f, 0.0f, 0.0f), Vec3 centrePosB_ = Vec3(0.0f, 0.0f, 0.0f), Vec3 halfExtents_ = Vec3(0.0f, 0.0f, 0.0f), Quaternion orientation_ = Quaternion(1.0f, Vec3(0.0f, 0.0f, 0.0f)));

	bool OnCreate() { return true; }
	void OnDestroy() {}
	void Update(const float deltaTime_) {}
	void Render()const {}
};

#pragma once
#include "TransformComponent.h"

using namespace MATH;

// There are two ways to set all this up, I'm going with option 2
// 1. I create indiviual collision components for each different collider type
// 2. I create one collision component that holds all data for all different collider types,
// then in engine, when adding a component, from the inspector window the user can decide which
// collider they want for the specific object, and it'll show the specific variables for that
// The benefits of option 2 is that it simplifies alot of things, i.e XML component save & load

// TODO: Inspector window, switch different states
enum class ColliderState {
	Discrete,
	Continuous
	// ContinuousDynamic (will implment later if issues with fast moving objects)
};

// Different collider types
// TODO: Inspector window, switch different types
enum class ColliderType {
	Sphere,
	Capsule,
	AABB,
	OBB
};

class CollisionComponent : public Component {
	friend class CollisionSystem;
	CollisionComponent(const CollisionComponent&) = delete;
	CollisionComponent(CollisionComponent&&) = delete;
	CollisionComponent& operator = (const CollisionComponent&) = delete;
	CollisionComponent& operator = (CollisionComponent&&) = delete;
protected:
	ColliderState colliderState = ColliderState::Discrete;
	ColliderType colliderType = ColliderType::Sphere; 

	bool isTrigger = false; // determines the type of response 

	float radius; /// Sphere, Capsule
	Vec3 centre; // Sphere, AABB, OBB
	Vec3 centrePosA; // Capsule
	Vec3 centrePosB; // Capsule
	Vec3 halfExtents; /// AABB, OBB
	Quaternion orientation; // OBB 

public:
	// Local vs World Coords
	// all the variables can be exposed locally in the inspector or in scripts
	// but they all need to be converted to world coords when doing calculations
	// this is similar to how unity does it where it has the exposed local variables,
	// and in editor when you change the actors pos, scale, rotation etc, the collider follows suit, 
	// but the local variables remain the same
	Vec3 getWorldCentre(Ref<TransformComponent> transform_) const { return Vec3(transform_->GetTransformMatrix() * Vec4(centre, 1.0f)); }
	float getWorldRadius(Ref<TransformComponent> transform_) const {
		Vec3 scale = transform_->GetScale();
		float maxScale = std::max(std::max(scale.x, scale.y), scale.z);
		return radius * maxScale;
	}
	Vec3 getWorldCentrePosA(Ref<TransformComponent> transform_) const { return Vec3(transform_->GetTransformMatrix() * Vec4(centrePosA, 1.0f)); }
	Vec3 getWorldCentrePosB(Ref<TransformComponent> transform_) const { return Vec3(transform_->GetTransformMatrix() * Vec4(centrePosB, 1.0f)); }
	Vec3 getWorldHalfExtents(Ref<TransformComponent> transform_) const {
		Vec3 scale = transform_->GetScale();
		return Vec3(halfExtents.x * scale.x, halfExtents.y * scale.y, halfExtents.z * scale.z);
	}
	Quaternion getWorldOrientation(Ref<TransformComponent> transform_) const { return transform_->GetOrientation() * orientation; }


public:
	CollisionComponent(Component* parent_ = nullptr, ColliderState state_ = ColliderState::Discrete, ColliderType type_ = ColliderType::Sphere, bool isTrigger_ = false, float radius_ = 1.0f, Vec3 centre_ = Vec3(0.0f, 0.0f, 0.0f),
		Vec3 centrePosA_ = Vec3(0.0f, 1.0f, 0.0f), Vec3 centrePosB_ = Vec3(0.0f, -1.0f, 0.0f), Vec3 halfExtents_ = Vec3(1.0f, 1.0f, 1.0f), Quaternion orientation_ = Quaternion(1.0f, Vec3(0.0f, 0.0f, 0.0f)));

	bool OnCreate() { return true; }
	void OnDestroy() {}
	void Update(const float deltaTime_) {}
	void Render()const {}

	// getters
	ColliderState getState() const { return colliderState; }
	ColliderType getType() const { return colliderType; }
	bool getIsTrigger() const { return isTrigger; }
	float getRadius() const { return radius; }
	Vec3 getCentre() const { return centre; }
	Vec3 getCentrePosA() const { return centrePosA; }
	Vec3 getCentrePosB() const { return centrePosB; }
	Vec3 getHalfExtents() const { return halfExtents; }
	Quaternion getOrientation() const { return orientation; }

	// setters
	void setState(ColliderState state_) { colliderState = state_;}
	void setType(ColliderType type_) { colliderType = type_; }
	void setIsTrigger(bool trigger_) { isTrigger = trigger_; }
	void setRadius(float radius_) { radius = radius_; }
	void setCentre(Vec3 centre_) { centre = centre_; }
	void setCentrePosA(Vec3 centreA_) { centrePosA = centreA_; }
	void setCentrePosB(Vec3 centreB_) { centrePosB = centreB_; }
	void setHalfExtents(Vec3 halfExtents_) { halfExtents = halfExtents_; }
	void setOrientation(Quaternion ori_) { orientation = ori_; }
};

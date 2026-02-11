#pragma once
#include "Actor.h"

using namespace MATH;

// reworking physics component to now only hold physics data instead of also trying to handle actual physics interactions
// now it is more simialr to how the collisioncomponent works
// the components just store information, and the systems handle the functions/interactions

// using unitys rigidbody component as a reference for the physics component

enum class PhysicsState {
	Dynamic,
	Kinematic,
	Static
};

class PhysicsComponent : public Component
{
	friend class PhysicsSystem;
	PhysicsComponent(const PhysicsComponent&) = delete;
	PhysicsComponent(PhysicsComponent&&) = delete;
	PhysicsComponent& operator = (const PhysicsComponent&) = delete;
	PhysicsComponent& operator = (PhysicsComponent&&) = delete;

private:
	PhysicsState state;
	float mass;
	bool useGravity;
	float drag;
	float angularDrag;

	// TODO: add rest of physics, inspector, and XML code
	// used in the collsion system for contact resolution
	float friction = 0.5f;
	float restitution = 0.0f;

	// linear motion 
	Vec3 vel;
	Vec3 accel;	
	// angular motion (setting this up just for now, if we want to do angular stuff after)
	Vec3 angularVel;
	Vec3 angularAcc;
	
	//Matrix3 rotationalInertia;
	
	// constraints (more so for freezing position and rotation, will implement actual constraints later if we want them)

public:
	PhysicsComponent(Component* parent_ = nullptr, PhysicsState state_ = PhysicsState::Dynamic, float mass_ = 1.0f, 
		bool useGrav_ = true, float drag_ = 0.0f, float angularDrag_ = 0.05f);
	~PhysicsComponent() {};

	bool OnCreate() { return true; }
	void OnDestroy() {};
	void Update(const float deltaTime) {};
	void Render() const {};

	// setters
	void setState(PhysicsState state_) { state = state_; }
	void setMass(const float& mass_) { mass = mass_; } 
	void setUseGravity(bool useGrav_) { useGravity = useGrav_; }

	void setDrag(float drag_) { drag = drag_; }
	void setAngularDrag(float angDrag_) { angularDrag = angDrag_; }

	void setFriction(float friction_) { friction = friction_; }
	void setRestitution(float restitution_) { restitution = restitution_; }

	void setVel(const Vec3& vel_) { vel = vel_; }
	void setAccel(const Vec3& accel_) { accel = accel_; }
	void setAngularVel(const Vec3& angularVel_) { angularVel = angularVel_; }
	void setAngularAccel(const Vec3& angularAccel_) { angularAcc = angularAccel_; }

	// getters
	PhysicsState getState() const { return state; }
	float getMass() const { return (state == PhysicsState::Static) ? FLT_MAX : mass; } // if static, return infinite mass
	bool getUseGravity() const { return useGravity; }

	float getDrag() const { return drag; }
	float getAngularDrag() const { return angularDrag; }

	float getFriction() const { return friction; }
	float getRestitution() const { return restitution; }

	Vec3 getVel() { return vel; }
	Vec3 getAccel() { return accel; }
	Vec3 getAngularVel() { return angularVel; }
	Vec3 getAngularAccel() { return angularAcc; }
};


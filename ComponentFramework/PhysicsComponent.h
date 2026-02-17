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

// https://docs.unity3d.com/6000.0/Documentation/ScriptReference/RigidbodyConstraints.html
struct PhysicsConstraints {
	bool freezePosX = false, freezePosY = false, freezePosZ = false;
	bool freezeRotX = false, freezeRotY = false, freezeRotZ = false;
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

	// used in the collsion system for contact resolution
	float friction;
	float restitution;

	// linear motion 
	Vec3 vel;
	Vec3 accel;	
	// angular motion (setting this up just for now, if we want to do angular stuff after)
	Vec3 angularVel;
	Vec3 angularAcc;
	
	// fixing the acceleration applying, instead of manually setting it (which casued only the last force to applied)
	// basically accumulate all the forces, then apply them at the end
	Vec3 forceAccumulator;
	Vec3 torqueAccumulator;

	//Matrix3 rotationalInertia;
	
	PhysicsConstraints constraints;

public:
	PhysicsComponent(Component* parent_ = nullptr, PhysicsState state_ = PhysicsState::Dynamic, PhysicsConstraints constraints_ = PhysicsConstraints(), float mass_ = 1.0f,
		bool useGrav_ = true, float drag_ = 0.0f, float angularDrag_ = 0.05f, float friction_ = 0.5f, float restitution_ = 0.0f);
	~PhysicsComponent() {};

	bool OnCreate() { return true; }
	void OnDestroy() {};
	void Update(const float deltaTime) {};
	void Render() const {};

	void ClearAccumulators() {
		forceAccumulator = Vec3(0.0f, 0.0f, 0.0f);
		torqueAccumulator = Vec3(0.0f, 0.0f, 0.0f);
	}

	// no longer doing apply force, instead add force to the accumalator
	void AddForce(const Vec3& force) { forceAccumulator += force; }
	void AddTorque(const Vec3& torque) { torqueAccumulator += torque; }

	// for constraints
	Vec3 ApplyPositionConstraints(const Vec3& vector_) {
		Vec3 result = vector_;
		if (constraints.freezePosX) result.x = 0.0f;
		if (constraints.freezePosY) result.y = 0.0f;
		if (constraints.freezePosZ) result.z = 0.0f;
		return result;
	}
	Vec3 ApplyRotationConstraints(const Vec3& vector_) {
		Vec3 result = vector_;
		if (constraints.freezeRotX) result.x = 0.0f;
		if (constraints.freezeRotY) result.y = 0.0f;
		if (constraints.freezeRotZ) result.z = 0.0f;
		return result;
	}

	// setters
	void setState(PhysicsState state_) { state = state_; }
	void setMass(const float& mass_) { mass = mass_; } 
	void setUseGravity(bool useGrav_) { useGravity = useGrav_; }

	void setDrag(float drag_) { drag = drag_; }
	void setAngularDrag(float angDrag_) { angularDrag = angDrag_; }

	void setFriction(float friction_) { friction = friction_; }
	void setRestitution(float restitution_) { restitution = restitution_; }

	void setVel(const Vec3& vel_) { vel = ApplyPositionConstraints(vel_); }
	void setAccel(const Vec3& accel_) { accel = ApplyPositionConstraints(accel_); }
	void setAngularVel(const Vec3& angularVel_) { angularVel = ApplyRotationConstraints(angularVel_); }
	void setAngularAccel(const Vec3& angularAccel_) { angularAcc = ApplyRotationConstraints(angularAccel_); }

	void setConstraints(const PhysicsConstraints& constraints_) { constraints = constraints_; }


	// getters
	PhysicsState getState() const { return state; }
	float getMass() const { return (state == PhysicsState::Static) ? FLT_MAX : mass; } // if static, return infinite mass
	float getInverseMass() const { return (state == PhysicsState::Dynamic && mass > VERY_SMALL) ? 1.0f / mass : 0.0f; } // getter for inverse mass
	bool getUseGravity() const { return useGravity; }

	float getDrag() const { return drag; }
	float getAngularDrag() const { return angularDrag; }

	float getFriction() const { return friction; }
	float getRestitution() const { return restitution; }

	Vec3 getVel() { return vel; }
	Vec3 getAccel() { return accel; }
	Vec3 getAngularVel() { return angularVel; }
	Vec3 getAngularAccel() { return angularAcc; }

	const PhysicsConstraints& getConstraints() const { return constraints; }
};


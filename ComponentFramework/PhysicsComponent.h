#pragma once
#include "Component.h"
#include "Actor.h"
#include <Vector.h>
using namespace MATH;

// converted Body into a component 

class PhysicsComponent : public Component
{
	// delete the move and copy constructers
	PhysicsComponent(const PhysicsComponent&) = delete;
	PhysicsComponent(PhysicsComponent&&) = delete;
	PhysicsComponent& operator = (const PhysicsComponent&) = delete;
	PhysicsComponent& operator = (PhysicsComponent&&) = delete;

private:
	Vec3 vel;
	Vec3 accel;
	float mass;

public:
	PhysicsComponent(Component* parent_, float mass_);
	~PhysicsComponent();

	bool OnCreate() override;
	void OnDestroy() override;
	void Update(float deltaTime) override;
	void Render() const;
	
	void ApplyForce(Vec3 force);

	// cause its inherting from a pure virtual I cant modify Update, so new function
	void UpdateP(float deltaTime, Ref<Actor> actor);

	// setters
	void setVel(const Vec3& vel_) { vel = vel_; }
	void setAccel(const Vec3& accel_) { accel = accel_; }
	void setMass(const float& mass_) { mass = mass_; }

	// getters
	Vec3 getVel() { return vel; }
	Vec3 getAccel() { return accel; }
	float getMass() const { return mass; }
};


#pragma once
#include "PhysicsComponent.h"
#include "Actor.h"

class PhysicsSystem
{
private:
	std::vector<Ref<Actor>> physicsActors;

	// gravity coefficent
	float gravity = -9.81f;

	// deleting copy and move constructers, setting up singleton
	PhysicsSystem() = default;
	PhysicsSystem(const PhysicsSystem&) = delete;
	PhysicsSystem(PhysicsSystem&&) = delete;
	PhysicsSystem& operator=(const PhysicsSystem&) = delete;
	PhysicsSystem& operator=(PhysicsSystem&&) = delete;
public:
	// Meyers Singleton (from JPs class)
	static PhysicsSystem& getInstance() {
		static PhysicsSystem instance;
		return instance;
	}

	void AddActor(Ref<Actor> actor_);
	void RemoveActor(Ref<Actor> actor_);

	void ClearActors() { physicsActors.clear(); }

	// Physics Functions
	void Update(float deltaTime);

	void UpdatePos(Ref<Actor> actor_, float deltaTime);
	void UpdateVel(Ref<Actor> actor_, float deltaTime);
	void UpdateOrientation(Ref<Actor> actor_, float deltaTime);

	// since physics system is a singleton, these function could be called outside of the physics system itself,
	// so additional checks are needed for these functions just incase 
	void ApplyForce(Ref<Actor> actor_, const Vec3& force);
	// similar to apply force but instead of it affecting the acceleration to affects the velocity
	void ApplyImpulse(Ref<Actor> actor_, const Vec3& impulse); 
	
	// resets physics system, sets all velocitys and accelerations to 0
	void ResetPhysics();

	// TODO: things to add in future if needed
	//void ApplyTorque(Ref<Actor> actor_, const Vec3& torque); need rotational inertia
	// creating constraints (mostly likely will be their own components)

	// getters and setters
	float getGravity() const { return gravity; }
	void setGravity(float gravity_) { gravity = gravity_; }
};


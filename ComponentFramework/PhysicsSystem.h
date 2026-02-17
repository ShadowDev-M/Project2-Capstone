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
	void UpdateVel(Ref<Actor> actor_, float deltaTime);
	void UpdatePos(Ref<Actor> actor_, float deltaTime);
	void UpdateOrientation(Ref<Actor> actor_, float deltaTime);
	
	// resets physics system, sets all velocitys and accelerations to 0
	void ResetPhysics();

	// getters and setters
	float getGravity() const { return gravity; }
	void setGravity(float gravity_) { gravity = gravity_; }
};


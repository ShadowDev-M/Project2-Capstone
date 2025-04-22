#pragma once
#include <vector>
#include "CollisionComponent.h"
#include "PhysicsComponent.h"
#include "Actor.h"
#include "Debug.h"
#include <Sphere.h>
#include <VMath.h>
#include <PMath.h>
using namespace MATH;
using namespace MATHEX;

class CollisionSystem {
private:
	std::vector<Ref<Actor>> collidingActors;

	// collision response functions
	void SphereSphereCollisionResponse(Ref<Actor> s1, Ref<Actor> s2);
	void AABBAABBCollisionResponse(Ref<Actor> bb1, Ref<Actor> bb2);
	void SpherePlaneCollisionResponse(Ref<Actor> s1, Ref<Actor> p1);

public:
	/// This function will check the the actor being added is new and has the all proper components 
	void AddActor(Ref<Actor> actor_) {
		if (actor_->GetComponent<CollisionComponent>().get() == nullptr) {
			Debug::Error("The Actor must have a CollisionComponent - ignored ", __FILE__, __LINE__);
			return;
		}

		if (actor_->GetComponent<PhysicsComponent>().get() == nullptr) {
			Debug::Error("The Actor must have a PhysicsComponent - ignored ", __FILE__, __LINE__);
			return;
		}
		collidingActors.push_back(actor_);
	}

	bool CollisionDetection(const Sphere& s1, const Sphere& s2) const;
	bool CollisionDetection(const AABB& bb1, const AABB& bb2) const;
	bool CollisionDetection(const Sphere s1, const Plane p1) const;
	void Update(const float deltaTime);

};

#pragma once
#include "CollisionComponent.h"
#include "PhysicsComponent.h"
#include "Actor.h"

using namespace MATH;

class CollisionSystem {
private:
	std::vector<Ref<Actor>> collidingActors;

	// collision response functions,
	// called in update when collision is detected
	
	/*
	void CollisionResponse(Ref<Actor> sphere1, Ref<Actor> sphere2);
	void CollisionResponse(Ref<Actor> sphere_, Ref<Actor> cylinder_);
	void CollisionResponse(Ref<Actor> sphere_, Ref<Actor> capsule_);
	void CollisionResponse(Ref<Actor> sphere_, Ref<Actor> aabb_);
	void CollisionResponse(Ref<Actor> sphere_, Ref<Actor> obb_);
	void CollisionResponse(Ref<Actor> cylinder1, Ref<Actor> cylinder2);
	void CollisionResponse(Ref<Actor> cylinder_, Ref<Actor> capsule_);
	void CollisionResponse(Ref<Actor> cylinder_, Ref<Actor> aabb_);
	void CollisionResponse(Ref<Actor> cylinder_, Ref<Actor> obb_);
	void CollisionResponse(Ref<Actor> capsule1, Ref<Actor> capsule2);
	void CollisionResponse(Ref<Actor> capsule_, Ref<Actor> aabb_);
	void CollisionResponse(Ref<Actor> capsule_, Ref<Actor> obb_);
	void CollisionResponse(Ref<Actor> aabb1, Ref<Actor> aabb2);
	void CollisionResponse(Ref<Actor> aabb_, Ref<Actor> obb_);
	void CollisionResponse(Ref<Actor> obb1, Ref<Actor> obb2);
	*/

public:
	/// This function will check the actor being added is new and has the all proper components 
	void AddActor(Ref<Actor> actor_);
	void RemoveActor(Ref<Actor> actor_);


	// collision detection functions,
	
	/*
	void CollisionDetection(const Sphere& sphere1, const Sphere& sphere2);
	void CollisionDetection(const Sphere& sphere_, const Cylinder& cylinder_);
	void CollisionDetection(const Sphere& sphere_, const Capsule& capsule_);
	void CollisionDetection(const Sphere& sphere_, const AABB& aabb_);
	void CollisionDetection(const Sphere& sphere_, const OBB& obb_);
	void CollisionDetection(const Cylinder& cylinder1, const Cylinder& cylinder2);
	void CollisionDetection(const Cylinder& cylinder_, const Capsule& capsule_);
	void CollisionDetection(const Cylinder& cylinder_, const AABB& aabb_);
	void CollisionDetection(const Cylinder& cylinder_, const OBB& obb_);
	void CollisionDetection(const Capsule& capsule1, const Capsule& capsule2);
	void CollisionDetection(const Capsule& capsule_, const AABB& aabb_);
	void CollisionDetection(const Capsule& capsule_, const OBB& obb_);
	void CollisionDetection(const AABB& aabb1, const AABB& aabb2);
	void CollisionDetection(const AABB& aabb_, const OBB& obb_);
	void CollisionDetection(const OBB& obb1, const OBB& obb2);
	*/
	
	
	// raycasting functions
	//Ref<Actor> PhysicsRaycast(Vec3 start, Vec3 end);
	 
	void Update(const float deltaTime);

};

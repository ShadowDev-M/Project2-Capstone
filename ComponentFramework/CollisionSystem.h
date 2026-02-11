#pragma once
#include "CollisionComponent.h"
#include "PhysicsComponent.h"
#include "Actor.h"
#include <set>

using namespace MATH;

// holds all the information about a given collision 
// Game Physics Engine Development Ian Millington 13.2.2
struct CollisionData {
	bool isColliding = false;
	Vec3 contactPoint; // Holds the position of the contact in world coordinates.
	Vec3 contactNormal; // Holds the direction of the contact in world coordinates.
	float penetration = 0.0f;
	float timeOfImpact = 0.0f; // for continous detection
	Vec3 relVel;
};

// struct for raycasting, holds all informationn about a raycast
// mostly based off unity, (I stole the name from unity) and old physics raycast code
// https://docs.unity3d.com/6000.0/Documentation/ScriptReference/RaycastHit.html
struct RaycastHit {
	bool didHit = false;
	Ref<CollisionComponent> hitCollider = nullptr; // return the hit collider, might as well 
	Ref<Actor> hitActor = nullptr; // unity returns specific components of the hit collider, but why not just return the actor too
	float distance = 0.0f; // The distance from the ray's origin to the impact point.
	Vec3 normal; // The normal of the surface the ray hit.
	Vec3 point; // The impact point in world space where the ray hit the collider.
};

// this is mostly to help out with collision detection event functions,
// like OnCollisionEnter/OnCollisionExit, this will keep track of the collided actors current state
struct CollidedPair {
	// id pair of the actors 
	uint32_t idA;
	uint32_t idB;

	// have to store whatever has the smallest id first
	CollidedPair(uint32_t a, uint32_t b) {
		idA = std::min(a, b);
		idB = std::max(a, b);
	}

	bool operator<(const CollidedPair& other) const {
		if (idA != other.idA) return idA < other.idA;
		return idB < other.idB;
	}

	bool operator==(const CollidedPair& other) const {
		return idA == other.idA && idB == other.idB;
	}
};

class CollisionSystem {
public:
	// Meyers Singleton (from JPs class)
	static CollisionSystem& getInstance() {
		static CollisionSystem instance;
		return instance;
	}

	/// This function will check the actor being added is new and has the all proper components 
	void AddActor(Ref<Actor> actor_);
	void RemoveActor(Ref<Actor> actor_);
	void ClearActors();
	void Update(const float deltaTime);

private:
	// deleting copy and move constructers, setting up singleton
	CollisionSystem() = default;
	CollisionSystem(const CollisionSystem&) = delete;
	CollisionSystem(CollisionSystem&&) = delete;
	CollisionSystem& operator=(const CollisionSystem&) = delete;
	CollisionSystem& operator=(CollisionSystem&&) = delete;

	std::vector<Ref<Actor>> collidingActors;
	// to help keep track of colliding actors for onhit event functions
	std::set<CollidedPair> currentCollidingActors;
	std::set<CollidedPair> previousCollidingActors;
	
	// continous detection checks between frames, and sudo-raycasts to look at whats ahead,
	// so deltatime is needed to see what is ahead before it actually happens in Update
	float deltaTime = 0.0f; 

public: 
	// main collision detection function,
	// will delegate which collisiond are detected by checking collider type and state
	// and will call response/resolution functions as needed
	bool CollisionDetection(Ref<Actor> actor1_, Ref<Actor> actor2_, CollisionData& data);

	//TODO: add actor tags or some way of filtering out certain actors,
	// I mean technically we can just get the name of the hit actor and use that as a filter
	
	// main raycast function (using unity as reference https://docs.unity3d.com/6000.0/Documentation/ScriptReference/Physics.Raycast.html)
	// returns whatever was hit first
	RaycastHit Raycast(const Vec3& origin, const Vec3& direction, float maxDistance = FLT_MAX);
	// returns all hits
	std::vector<RaycastHit> RaycastAll(const Vec3& origin, const Vec3& direction, float maxDistance = FLT_MAX);

	// collision and trigger on enter/stay/exit functions (scripting)
	// not sure if these should be public or private, just change it to whatever is needed for scripting
	// these functions will be called in collision systems update
	// the actual functions themselves won't have much actual code, 
	// they will just be callbacks for the scripts, then collsiion systems Update calls them
	// can rename if needed
	// void OnCollisionEnter(Collision collision) / void OnCollisionExit(Collision collisionInfo)
	// Update will give the functions the actors and data associtated with the collision 
	void OnCollisionEnter(Ref<Actor> actor1_, Ref<Actor> actor2_, const CollisionData& data_);
	void OnCollisionStay(Ref<Actor> actor1_, Ref<Actor> actor2_, const CollisionData& data_);
	void OnCollisionExit(Ref<Actor> actor1_, Ref<Actor> actor2_, const CollisionData& data_);
	void OnTriggerEnter(Ref<Actor> actor1_, Ref<Actor> actor2_, const CollisionData& data_);
	void OnTriggerStay(Ref<Actor> actor1_, Ref<Actor> actor2_, const CollisionData& data_);
	void OnTriggerExit(Ref<Actor> actor1_, Ref<Actor> actor2_, const CollisionData& data_);
	
private:
	// rest of the collision detection functions
	bool SphereSphereDiscrete(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data);
	bool SphereSphereContinuous(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data);
	bool CapsuleCapsuleDiscrete(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data);
	bool CapsuleCapsuleContinuous(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data);
	bool AABBAABBDiscrete(Ref<Actor> aabb1, Ref<Actor> aabb2, CollisionData& data);
	bool OBBOBBDiscrete(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data);
	
	bool SphereCapsuleDiscrete(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data);
	bool SphereCapsuleContinuous(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data);
	bool SphereAABBDiscrete(Ref<Actor> s, Ref<Actor> aabb, CollisionData& data);
	bool SphereAABBContinuous(Ref<Actor> s, Ref<Actor> aabb, CollisionData& data);
	bool SphereOBBDiscrete(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data);
	bool SphereOBBContinuous(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data);
	bool CapsuleAABBDiscrete(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data);
	bool CapsuleAABBContinuous(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data);
	bool CapsuleOBBDiscrete(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data);
	bool CapsuleOBBContinuous(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data);
	bool AABBOBBDiscrete(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data);

	// collision resolution
	void ResolveCollision(Ref<Actor> actor1_, Ref<Actor> actor2_, const CollisionData& data);
	void ResolvePenetration(Ref<Actor> actor1_, Ref<Actor> actor2_, const CollisionData& data);
	void ResolveImpulse(Ref<Actor> actor1_, Ref<Actor> actor2_, const CollisionData& data);
	void ApplyFriction(Ref<Actor> actor1_, Ref<Actor> actor2_, const CollisionData& data, float impulse_);


	// shape raycast functions (reusing raycast code from last semester)
	// this is not like unitys SphereCast function,
	// that casts a sphere along a ray and checks for collisions
	// these functions instead just cast a ray out, 
	// and then determines what type of collider/shape they hit based on the function
	Ref<Actor> RaycastSphere(const Vec3& origin, const Vec3& direction, Ref<Actor> actor_, RaycastHit& hit);
	Ref<Actor> RaycastCapsule(const Vec3& origin, const Vec3& direction, Ref<Actor> actor_, RaycastHit& hit);
	Ref<Actor> RaycastAABB(const Vec3& origin, const Vec3& direction, Ref<Actor> actor_, RaycastHit& hit);
	Ref<Actor> RaycastOBB(const Vec3& origin, const Vec3& direction, Ref<Actor> actor_, RaycastHit& hit);	
	

	// helper functions from Real-Time Collision Detetcion book
	
	Vec3 ClosestPtPointAABB(const Vec3& p, const Vec3& aabbMin, const Vec3& aabbMax);
	
	// struct for raybox intersection
	struct RayAABBIntersection {
		bool didIntersect = false;
		float tMin = 0.0f;
		float tMax = 0.0f;
		int hitAxis = -1;
		bool hitMaxFace = false;
	};
	
	// also works for OBBs
	RayAABBIntersection IntersectRayAABB(const Vec3& rayOri_, const Vec3& rayDir_, const Vec3& boxMin_, const Vec3& boxMax_, float maxDistance = FLT_MAX);
};

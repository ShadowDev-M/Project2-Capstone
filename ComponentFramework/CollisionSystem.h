#pragma once
#include "CollisionComponent.h"
#include "PhysicsComponent.h"
#include "Actor.h"
#include <set>

using namespace MATH;


enum class CollisionDetectionState {
	Enter,
	Stay,
	Exit,
	TriggerEnter,
	TriggerStay,
	TriggerExit
};

// holds all the information about a given collision 
// used in collision resolution system
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
	bool isIntersected = false;
	Ref<CollisionComponent> hitCollider = nullptr; // return the hit collider, might as well 
	Ref<Actor> hitActor = nullptr; // unity returns specific components of the hit collider, but why not just return the actor too
	float t = 0.0f; // The distance from the ray's origin to the impact point.
	Vec3 normal; // The normal of the surface the ray hit.
	Vec3 intersectionPoint; // The impact point in world space where the ray hit the collider.
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
	
	// main raycast function (using unity as reference https://docs.unity3d.com/6000.0/Documentation/ScriptReference/Physics.Raycast.html)
	// returns whatever was hit first
	RaycastHit Raycast(const Vec3& origin, const Vec3& direction, float maxDistance = FLT_MAX);
	// returns all hits
	std::vector<RaycastHit> RaycastAll(const Vec3& origin, const Vec3& direction, float maxDistance = FLT_MAX);

	// will raycast with screen coords and return hit data
	RaycastHit ScreenRaycast(int sdlMouseX, int sdlMouseY);
	
private:
	// rest of the collision detection functions
	bool SphereSphereDiscrete(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data);
	bool SphereSphereContinuous(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data);
	bool CapsuleCapsuleDiscrete(Ref<Actor> c1, Ref<Actor> c2, CollisionData& data);
	bool CapsuleCapsuleContinuous(Ref<Actor> c1, Ref<Actor> c2, CollisionData& data);
	bool AABBAABBDiscrete(Ref<Actor> aabb1, Ref<Actor> aabb2, CollisionData& data);
	bool OBBOBBDiscrete(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data);
	
	bool SphereCapsuleDiscrete(Ref<Actor> s, Ref<Actor> c, CollisionData& data);
	bool SphereCapsuleContinuous(Ref<Actor> s, Ref<Actor> c, CollisionData& data);
	bool SphereAABBDiscrete(Ref<Actor> s, Ref<Actor> aabb, CollisionData& data);
	bool SphereAABBContinuous(Ref<Actor> s, Ref<Actor> aabb, CollisionData& data);
	bool SphereOBBDiscrete(Ref<Actor> s, Ref<Actor> obb, CollisionData& data);
	bool SphereOBBContinuous(Ref<Actor> s, Ref<Actor> obb, CollisionData& data);
	bool CapsuleAABBDiscrete(Ref<Actor> c, Ref<Actor> ab, CollisionData& data);
	bool CapsuleAABBContinuous(Ref<Actor> c, Ref<Actor> ab, CollisionData& data);
	bool CapsuleOBBDiscrete(Ref<Actor> c, Ref<Actor> ob, CollisionData& data);
	bool CapsuleOBBContinuous(Ref<Actor> c, Ref<Actor> ob, CollisionData& data);
	bool AABBOBBDiscrete(Ref<Actor> ab, Ref<Actor> ob, CollisionData& data);

	// TODO: continous functions for aabbaabb and obbobb

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
	bool RaycastSphere(const Vec3& origin, const Vec3& direction, Ref<Actor> actor_, RaycastHit& hit);
	bool RaycastCapsule(const Vec3& origin, const Vec3& direction, Ref<Actor> actor_, RaycastHit& hit);
	bool RaycastAABB(const Vec3& origin, const Vec3& direction, Ref<Actor> actor_, RaycastHit& hit);
	bool RaycastOBB(const Vec3& origin, const Vec3& direction, Ref<Actor> actor_, RaycastHit& hit);	
	
	// raycast helper functions
	bool checkInfiniteCylinder(const Vec3& origin, const Vec3& direction, Ref<Actor> actor_, RaycastHit& hit);
	bool checkEndSphere(const Vec3& origin, const Vec3& direction, const Vec3& axisDirection, Ref<Actor> actor_, RaycastHit& hit);

	// helper functions from Real-Time Collision Detetcion book
	float ClosestPtSegmentSegment(Vec3 p1, Vec3 q1, Vec3 p2, Vec3 q2, float& s, float& t, Vec3& c1, Vec3& c2);
	Vec3 ClosestPtPointSegment(Vec3 a, Vec3 b, Vec3 c);
	float SqDistPointSegment(Vec3 a, Vec3 b, Vec3 c);
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

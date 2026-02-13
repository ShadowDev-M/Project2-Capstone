#include "pch.h"
#include "CollisionSystem.h"

void CollisionSystem::AddActor(Ref<Actor> actor_) {
	if (actor_->GetComponent<CollisionComponent>().get() == nullptr) {
		Debug::Error("The Actor must have a CollisionComponent - ignored ", __FILE__, __LINE__);
		return;
	}

	if (actor_->GetComponent<PhysicsComponent>().get() == nullptr) {
		Debug::Error("The Actor must have a PhysicsComponent - ignored ", __FILE__, __LINE__);
		return;
	}

	// additional check to make sure not adding duplicate actors
	if (std::find(collidingActors.begin(), collidingActors.end(), actor_) != collidingActors.end()) {
		Debug::Warning("Actor already added to CollisionSystem", __FILE__, __LINE__);
		return;
	}

	collidingActors.push_back(actor_);
}

void CollisionSystem::RemoveActor(Ref<Actor> actor_) {
	// finding actor from vector and removing it
	auto it = std::find(collidingActors.begin(), collidingActors.end(), actor_);
	if (it != collidingActors.end()) {
		collidingActors.erase(it);
	}

	// removing collison pairs
	auto pairIt = currentCollidingActors.begin();
	while (pairIt != currentCollidingActors.end()) {
		if (pairIt->idA == actor_->getId() || pairIt->idB == actor_->getId()) {
			pairIt = currentCollidingActors.erase(pairIt);
		}
		else {
			pairIt++;
		}
	}
}

void CollisionSystem::ClearActors() {
	collidingActors.clear();
	//currentCollidingActors.clear();
	//previousCollidingActors.clear();
}

bool CollisionSystem::CollisionDetection(Ref<Actor> actor1_, Ref<Actor> actor2_, CollisionData& data)
{
	Ref<CollisionComponent> CC1 = actor1_->GetComponent<CollisionComponent>();
	Ref<CollisionComponent> CC2 = actor2_->GetComponent<CollisionComponent>();
	
	if (!CC1 || !CC2) return false;

	Ref<PhysicsComponent> PC1 = actor1_->GetComponent<PhysicsComponent>();
	Ref<PhysicsComponent> PC2 = actor2_->GetComponent<PhysicsComponent>();

	// if anyone of the colliders states are continuous then continuous detection will be used
	bool isContinuous = false;
	if (PC1 && PC2) {
		isContinuous = CC1->getState() == ColliderState::Continuous || CC2->getState() == ColliderState::Continuous;
	}

	ColliderType type1 = CC1->getType();
	ColliderType type2 = CC2->getType();

	// instead of writing functions like SphereAABBDiscrete & AABBSphereDiscrete, just do a swap, 
	// swap everything so that one function can be used whether its a sphere-ABB or a AABB-sphere thats colliding
	bool swapped = false;
	if (type1 > type2) {
		std::swap(actor1_, actor2_);
		std::swap(type1, type2);
		std::swap(CC1, CC2);
		swapped = true;
	}

	bool collisionDetected = false;

	// a big if tree, going through all collider types, and checking for different states, then calls the functions accordingly
	if (type1 == ColliderType::Sphere && type2 == ColliderType::Sphere) {
		if (isContinuous) {
			collisionDetected = SphereSphereContinuous(actor1_, actor2_, data);
		}
		else {
			collisionDetected = SphereSphereDiscrete(actor1_, actor2_, data);
		}
	}
	// TODO: sphere-capsule
	else if (type1 == ColliderType::Sphere && type2 == ColliderType::AABB) {
		if (isContinuous) {
			collisionDetected = SphereAABBContinuous(actor1_, actor2_, data);
		}
		else {
			collisionDetected = SphereAABBDiscrete(actor1_, actor2_, data);
		}
	}
	else if (type1 == ColliderType::Sphere && type2 == ColliderType::OBB) {
		if (isContinuous) {
			collisionDetected = SphereOBBContinuous(actor1_, actor2_, data);
		}
		else {
			collisionDetected = SphereOBBDiscrete(actor1_, actor2_, data);
		}
	}
	else if (type1 == ColliderType::Capsule && type2 == ColliderType::Capsule) {
		if (isContinuous) {
			collisionDetected = CapsuleCapsuleContinuous(actor1_, actor2_, data);
		}
		else {
			collisionDetected = CapsuleCapsuleDiscrete(actor1_, actor2_, data);
		}
	}
	else if (type1 == ColliderType::AABB && type2 == ColliderType::AABB) {
		collisionDetected = AABBAABBDiscrete(actor1_, actor2_, data);
	}
	else if (type1 == ColliderType::OBB && type2 == ColliderType::OBB) {
		collisionDetected = OBBOBBDiscrete(actor1_, actor2_, data);
	}
	else if (type1 == ColliderType::AABB && type2 == ColliderType::OBB) {
		collisionDetected = AABBOBBDiscrete(actor1_, actor2_, data);
	}
	else {
		return false;
	}

	// if the actors were swapped, the normals have to be flipped
	// normal should always point from B-A
	if (swapped && collisionDetected) {
		data.contactNormal = data.contactNormal * -1.0f;
	}

	return collisionDetected;
}

void CollisionSystem::ResolveCollision(Ref<Actor> actor1_, Ref<Actor> actor2_, const CollisionData& data)
{
	// 14.4.1 The Collision Resolution Pipeline Ian Millington

	Ref<PhysicsComponent> PC1 = actor1_->GetComponent<PhysicsComponent>();
	Ref<PhysicsComponent> PC2 = actor2_->GetComponent<PhysicsComponent>();

	Ref<CollisionComponent> CC1 = actor1_->GetComponent<CollisionComponent>();
	Ref<CollisionComponent> CC2 = actor2_->GetComponent<CollisionComponent>();

	// if the colliders are triggers then no collision response gets applied to them
	if (CC1->getIsTrigger() || CC2->getIsTrigger()) return;

	if (!PC1 || !PC2) return;

	// first resolve penetration,
	// this will make it so objects are seperated from each and don't just continue to collide into each other
	ResolvePenetration(actor1_, actor2_, data);

	// then resolve impulses,
	// changes in velocity if they need to be made, friction, bounciness/restatuion etc
	ResolveImpulse(actor1_, actor2_, data);
}

void CollisionSystem::ResolvePenetration(Ref<Actor> actor1_, Ref<Actor> actor2_, const CollisionData& data)
{
	// 14.4.3 Resolving Penetration

	Ref<TransformComponent> TC1 = actor1_->GetComponent<TransformComponent>();
	Ref<TransformComponent> TC2 = actor2_->GetComponent<TransformComponent>();

	Ref<PhysicsComponent> PC1 = actor1_->GetComponent<PhysicsComponent>();
	Ref<PhysicsComponent> PC2 = actor2_->GetComponent<PhysicsComponent>();

	PhysicsState state1 = PC1->getState();
	PhysicsState state2 = PC2->getState();

	// setting inverse mass based on physics state 
	float inverseMass1 = (state1 == PhysicsState::Dynamic) ? 1.0f / PC1->getMass() : 0.0f;
	float inverseMass2 = (state2 == PhysicsState::Dynamic) ? 1.0f / PC2->getMass() : 0.0f;

	float totalInverseMass = inverseMass1 + inverseMass2;

	// static and kinematic actors will have an inversemass of 0,
	// they shouldn't have any resolution be applied to them
	if (totalInverseMass < VERY_SMALL) return;

	// multiplying the contact normal by the penetration value to get seperation value
	Vec3 seperationValue = data.contactNormal * data.penetration;

	// only dynamic physics actors penetration will be resolved
	if (state1 == PhysicsState::Dynamic) {
		float mass = inverseMass1 / totalInverseMass;
		Vec3 seperationAmmount = seperationValue * mass;
		
		Vec3 newPos = TC1->GetPosition() + seperationAmmount;
		TC1->SetPos(newPos.x, newPos.y, newPos.z);
	}

	if (state2 == PhysicsState::Dynamic) {
		float mass = inverseMass2 / totalInverseMass;
		Vec3 seperationAmmount = seperationValue * mass;

		Vec3 newPos = TC2->GetPosition() - seperationAmmount;
		TC2->SetPos(newPos.x, newPos.y, newPos.z);
	}
}

void CollisionSystem::ResolveImpulse(Ref<Actor> actor1_, Ref<Actor> actor2_, const CollisionData& data)
{
	// 14.4.4 Resolving Velocity

	Ref<PhysicsComponent> PC1 = actor1_->GetComponent<PhysicsComponent>();
	Ref<PhysicsComponent> PC2 = actor2_->GetComponent<PhysicsComponent>();

	PhysicsState state1 = PC1->getState();
	PhysicsState state2 = PC2->getState();

	// only dynamic objects have impulses applied to them
	if (state1 != PhysicsState::Dynamic && state2 != PhysicsState::Dynamic) return;

	Vec3 vel1 = PC1->getVel();
	Vec3 vel2 = PC2->getVel();
	Vec3 relVel = vel1 - vel2;

	// how fast the actors are moving towards or away from each other
	float velAlongNormal = VMath::dot(relVel, data.contactNormal);

	// if the actors are already seperated then theres no need to apply impulses
	if (velAlongNormal > 0.0f) return;

	float restitution1 = PC1->getRestitution();
	float restitution2 = PC2->getRestitution();

	// restitution 0-1, 0 no bounce, 1 full bounce
	// getting the min makes it so if one of them is inelsatic, then they both will be
	float restitution = std::min(restitution1, restitution2);

	// calculating impulse magnitude 
	// Coefficient of Restitution e=|ua-ub| / |vb-va|
	float inverseMass1 = (state1 == PhysicsState::Dynamic) ? 1.0f / PC1->getMass() : 0.0f;
	float inverseMass2 = (state2 == PhysicsState::Dynamic) ? 1.0f / PC2->getMass() : 0.0f;

	float impulseMagnitude = -(1.0f + restitution) * velAlongNormal;
	impulseMagnitude /= (inverseMass1 + inverseMass2);

	Vec3 impulse = data.contactNormal * impulseMagnitude;

	// applying the impulse
	if (state1 == PhysicsState::Dynamic) {
		Vec3 newVel = vel1 + impulse * inverseMass1;
		PC1->setVel(newVel);
	}
	if (state2 == PhysicsState::Dynamic) {
		Vec3 newVel = vel2 - impulse * inverseMass2;
		PC2->setVel(newVel);
	}

	// applying friction
	ApplyFriction(actor1_, actor2_, data, impulseMagnitude);
}

void CollisionSystem::ApplyFriction(Ref<Actor> actor1_, Ref<Actor> actor2_, const CollisionData& data, float impulse_)
{
	// 15.4.1 Friction as Impulses

	Ref<PhysicsComponent> PC1 = actor1_->GetComponent<PhysicsComponent>();
	Ref<PhysicsComponent> PC2 = actor2_->GetComponent<PhysicsComponent>();

	PhysicsState state1 = PC1->getState();
	PhysicsState state2 = PC2->getState();

	Vec3 vel1 = PC1->getVel();
	Vec3 vel2 = PC2->getVel();
	Vec3 relVel = vel1 - vel2;

	// v=rω tangent velocity is like the sliding of the actor, it also resists against friction
	Vec3 tangentVel = relVel - data.contactNormal * VMath::dot(relVel, data.contactNormal);
	float tangentSpeed = VMath::mag(tangentVel);

	// no motion
	if (tangentSpeed < VERY_SMALL) return;

	Vec3 tangentDir = tangentVel / tangentSpeed;

	// friction
	float friction1 = PC1->getFriction();
	float friction2 = PC2->getFriction();

	// geometric mean friction, provides more realistic friction, 
	// also helps when different actors have different friction values
	float friction = sqrt(friction1 * friction2);

	// calculating impulse magnitude 
	// Coefficient of Restitution e=|ua-ub| / |vb-va|
	float inverseMass1 = (state1 == PhysicsState::Dynamic) ? 1.0f / PC1->getMass() : 0.0f;
	float inverseMass2 = (state2 == PhysicsState::Dynamic) ? 1.0f / PC2->getMass() : 0.0f;
	
	float frictionImpulseMag = -tangentSpeed / (inverseMass1 + inverseMass2);

	// Coulomb’s Law of Friction
	// friction is proportional to the normal
	float maxFriction = abs(impulse_ * friction);
	frictionImpulseMag = std::max(-maxFriction, std::min(frictionImpulseMag, maxFriction));

	// applying friction
	Vec3 frictionImpulse = tangentDir * frictionImpulseMag;
	if (state1 == PhysicsState::Dynamic) {
		PC1->setVel(PC1->getVel() + frictionImpulse * inverseMass1);
	}
	if (state2 == PhysicsState::Dynamic) {
		PC2->setVel(PC2->getVel() - frictionImpulse * inverseMass2);
	}
}

void CollisionSystem::Update(float deltaTime) {
	// TODO: rework
	
	// pass deltatime
	this->deltaTime = deltaTime;

	// loop through all colliding actors, two times cause two objects colliding
	for (size_t i = 0; i < collidingActors.size(); i++) {
		for (size_t j = i + 1; j < collidingActors.size(); j++) {

			Ref<Actor> a1 = collidingActors[i];
			Ref<Actor> a2 = collidingActors[j];
			
			// get the transform components from the colliding actors
			Ref<TransformComponent> TC1 = collidingActors[i]->GetComponent<TransformComponent>();
			Ref<TransformComponent> TC2 = collidingActors[j]->GetComponent<TransformComponent>();

			// get the collision component from the colliding actors
			Ref<CollisionComponent> CC1 = a1->GetComponent<CollisionComponent>();
			Ref<CollisionComponent> CC2 = a2->GetComponent<CollisionComponent>();

			

			
				
				CollisionData data;

				// if collisison was detected between the two spheres, do response
				if (CollisionDetection(a1, a2, data)) {
					
					//printf("x: %f, y: %f, z: %f\n", data.contactNormal.x, data.contactNormal.y, data.contactNormal.z);

					//std::cout << data.penetration << std::endl;
					ResolveCollision(a1, a2, data);
					std::cout << "COLLSION DETECTED!" << std::endl;
				}
			
		}

	}
}

bool CollisionSystem::SphereSphereDiscrete(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data)
{
	// Real-Time Collision Detection Ericson 4.3.1

	Ref<TransformComponent> TC1 = s1->GetComponent<TransformComponent>();
	Ref<TransformComponent> TC2 = s2->GetComponent<TransformComponent>();

	Ref<CollisionComponent> CC1 = s1->GetComponent<CollisionComponent>();
	Ref<CollisionComponent> CC2 = s2->GetComponent<CollisionComponent>();

	// Calculate squared distance between centers
	Vec3 d = CC1->getWorldCentre(TC1) - CC2->getWorldCentre(TC2);
	float dist2 = VMath::dot(d, d);

	// Spheres intersect if squared distance is less than squared sum of radii
	float radiusSum = CC1->getWorldRadius(TC1) + CC2->getWorldRadius(TC2);
	if (dist2 <= (radiusSum * radiusSum)) {
		// Collision data
		// 13.3.1 Colliding Two Spheres Ian Millington
		data.isColliding = true;
		data.contactNormal = VMath::normalize(d);
		data.contactPoint = CC1->getWorldCentre(TC1) + d * VMath::mag(d) * 0.5f;
		data.penetration = radiusSum - VMath::mag(d);

		return true;
	}

	return false;
}

bool CollisionSystem::SphereSphereContinuous(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data)
{
	// Real-Time Collision Detection Ericson 5.5.5

	Ref<TransformComponent> TC1 = s1->GetComponent<TransformComponent>();
	Ref<TransformComponent> TC2 = s2->GetComponent<TransformComponent>();
	
	Ref<PhysicsComponent> PC1 = s1->GetComponent<PhysicsComponent>();
	Ref<PhysicsComponent> PC2 = s2->GetComponent<PhysicsComponent>();

	Ref<CollisionComponent> CC1 = s1->GetComponent<CollisionComponent>();
	Ref<CollisionComponent> CC2 = s2->GetComponent<CollisionComponent>();

	// getting previous position 
	Vec3 previousPosA = CC2->getWorldCentre(TC1) - (PC1->getVel() * deltaTime); // (displacement) d=vt
	Vec3 previousPosB = CC2->getWorldCentre(TC2) - (PC2->getVel() * deltaTime);
	
	// Subtract movement of s1 from both s0 and s1, making s1 stationary
	Vec3 v = PC1->getVel() - PC2->getVel();
	
	// divide by zero check (inital velocity will start at 0, also works as a fallback)
	if (VMath::mag(v) < VERY_SMALL) {
		return SphereSphereDiscrete(s1, s2, data);
	}
	
	// Expand sphere s1 by the radius of s0
	float r = CC2->getWorldRadius(TC2) + CC1->getWorldRadius(TC1);

	// Can now test directed segment s = s0.c + tv, v = (v0-v1)/||v0-v1|| against
	// the expanded sphere for intersection
	float vlen = VMath::mag(v);
	
	Vec3 relDir = v / vlen;

	Vec3 m = previousPosA - previousPosB;
	float a = VMath::dot(relDir, relDir);
	float b = 2.0f * VMath::dot(m, v / vlen);
	float c = VMath::dot(m, m) - (r * r);
	// Exit if r’s origin outside s (c > 0) and r pointing away from s (b > 0)
	if (c > 0.0f && b > 0.0f) return 0;
	float discr = b * b - 4.0f * a * c;
	// A negative discriminant corresponds to ray missing sphere
	if (discr < 0.0f) return false;
	// Ray now found to intersect sphere, compute smallest t value of intersection
	float t = (-b - sqrt(discr)) / (2.0f * a);
	
	// If t is negative, ray started inside sphere so clamp t to zero (fallback to discrete)
	if (t < 0.0f) {
		return SphereSphereDiscrete(s1, s2, data);
	}

	float timeOfImpact = t / vlen;

	// if the time of impact is greater than deltatime then the collision happens in a future frame
	if (timeOfImpact > deltaTime) {
		return false;
	}

	// Collision data
	data.isColliding = true;
	data.timeOfImpact = timeOfImpact / deltaTime; // normalizing

	// getting what the positions are at the time of the impact
	Vec3 posAtImpact1 = previousPosA + PC1->getVel() * timeOfImpact;
	Vec3 posAtImpact2 = previousPosB + PC2->getVel() * timeOfImpact;

	// getting normal
	Vec3 d = posAtImpact1 - posAtImpact2;

	// divide by 0 fallback
	if (VMath::mag(d) < VERY_SMALL) {
		data.contactNormal = relDir;
	}
	else {
		data.contactNormal = VMath::normalize(d);
	}

	data.contactPoint = posAtImpact2 + data.contactNormal * CC2->getWorldRadius(TC2);
	data.penetration = 0.0f; // penetration is at 0 since continous detects the collision right at the moment of impact
	data.relVel = v;

	return true;
}

bool CollisionSystem::CapsuleCapsuleDiscrete(Ref<Actor> c1, Ref<Actor> c2, CollisionData& data)
{
	// 4.5.1 Sphere-swept Volume Intersection

	Ref<TransformComponent> TC1 = c1->GetComponent<TransformComponent>();
	Ref<TransformComponent> TC2 = c2->GetComponent<TransformComponent>();
								  
	Ref<CollisionComponent> CC1 = c1->GetComponent<CollisionComponent>();
	Ref<CollisionComponent> CC2 = c2->GetComponent<CollisionComponent>();

	Vec3 p1 = CC1->getWorldCentrePosA(TC1);
	Vec3 q1 = CC1->getWorldCentrePosB(TC1);
	Vec3 p2 = CC2->getWorldCentrePosA(TC2);
	Vec3 q2 = CC2->getWorldCentrePosB(TC2);

	// Compute (squared) distance between the inner structures of the capsules
	float s, t;
	Vec3 C1, C2;
	float dist2 = ClosestPtSegmentSegment(p1, q1, p2, q2, s, t, C1, C2);

	// If (squared) distance smaller than (squared) sum of radii, they collide
	float radius = CC1->getWorldCapsuleRadius(TC1) + CC2->getWorldCapsuleRadius(TC2);
	if (dist2 <= (radius * radius)) {
		Vec3 d = C1 - C2;
		float dist = sqrt(dist2);

		data.isColliding = true;
		data.contactNormal = (dist > VERY_SMALL) ? VMath::normalize(d) : Vec3(0.0f, 1.0f, 0.0f);
		data.contactPoint = C2 + data.contactNormal * CC2->getWorldCapsuleRadius(TC2);
		data.penetration = radius - dist; 

		return true;
	}

	return false;
}

bool CollisionSystem::CapsuleCapsuleContinuous(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data)
{
	return false;
}

bool CollisionSystem::AABBAABBDiscrete(Ref<Actor> aabb1, Ref<Actor> aabb2, CollisionData& data)
{
	// Real-Time Collision Detection Ericson 4.2.1

	Ref<TransformComponent> TC1 = aabb1->GetComponent<TransformComponent>();
	Ref<TransformComponent> TC2 = aabb2->GetComponent<TransformComponent>();

	Ref<CollisionComponent> CC1 = aabb1->GetComponent<CollisionComponent>();
	Ref<CollisionComponent> CC2 = aabb2->GetComponent<CollisionComponent>();

	Vec3 minA = CC1->getWorldCentre(TC1) - CC1->getWorldHalfExtents(TC1);
	Vec3 maxA = CC1->getWorldCentre(TC1) + CC1->getWorldHalfExtents(TC1);
	Vec3 minB = CC2->getWorldCentre(TC2) - CC1->getWorldHalfExtents(TC2);
	Vec3 maxB = CC2->getWorldCentre(TC2) + CC1->getWorldHalfExtents(TC2);

	// Exit with no intersection if separated along an axis
	if (maxA.x < minB.x || minA.x > maxB.x) return false;
	if (maxA.y < minB.y || minA.y > maxB.y) return false;
	if (maxA.z < minB.z || minA.z > maxB.z) return false;
	// Overlapping on all axes means AABBs are intersecting
	data.isColliding = true;

	// Game Physics Engine Development Ian Millington 13.4.2 Colliding Two Boxes

	float overlapX = std::min(maxA.x - minB.x, maxB.x - minA.x);
	float overlapY = std::min(maxA.y - minB.y, maxB.y - minA.y);
	float overlapZ = std::min(maxA.z - minB.z, maxB.z - minA.z);

	// checking overlaps to get collision data
	if (overlapX < overlapY && overlapX < overlapZ) {
		data.penetration = overlapX;
		data.contactNormal = (CC1->getWorldCentre(TC1).x < CC2->getWorldCentre(TC2).x) ? Vec3(-1.0f, 0.0f, 0.0f) : Vec3(1.0f, 0.0f, 0.0f);
	}
	else if (overlapY < overlapZ) {
		data.penetration = overlapY;
		data.contactNormal = (CC1->getWorldCentre(TC1).y < CC2->getWorldCentre(TC2).y) ? Vec3(0.0f, -1.0f, 0.0f) : Vec3(0.0f, 1.0f, 0.0f);
	}
	else {
		data.penetration = overlapZ;
		data.contactNormal = (CC1->getWorldCentre(TC1).z < CC2->getWorldCentre(TC2).z) ? Vec3(0.0f, 0.0f, -1.0f) : Vec3(0.0f, 0.0f, 1.0f);
	}

	// which vertex would lie closest to the first box. This is the vertex it uses to generate the contact point
	Vec3 overlapMin(std::max(minA.x, minB.x), std::max(minA.y, minB.y), std::max(minA.z, minB.z));
	Vec3 overlapMax(std::min(minA.x, minB.x), std::min(minA.y, minB.y), std::min(minA.z, minB.z));
	data.contactPoint = (overlapMin + overlapMax) * 0.5f;

	return true;
}

bool CollisionSystem::OBBOBBDiscrete(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data)
{
	// 4.4.1 OBB-OBB Intersection Real-Time Collision Detection Ericson

	Ref<TransformComponent> TC1 = s1->GetComponent<TransformComponent>();
	Ref<TransformComponent> TC2 = s2->GetComponent<TransformComponent>();

	Ref<CollisionComponent> CC1 = s1->GetComponent<CollisionComponent>();
	Ref<CollisionComponent> CC2 = s2->GetComponent<CollisionComponent>();
	
	// setting up variables
	Vec3 centre1 = CC1->getWorldCentre(TC1);
	Vec3 halfExtents1 = CC1->getWorldHalfExtents(TC1);
	Quaternion ori1 = CC1->getWorldOrientation(TC1);
	// rotating the orienation around x,y,z axis in order to get local coords
	Vec3 axes1[3] = { 
		QMath::rotate(Vec3(1.0f, 0.0f, 0.0f), ori1),
		QMath::rotate(Vec3(0.0f, 1.0f, 0.0f), ori1),
		QMath::rotate(Vec3(0.0f, 0.0f, 1.0f), ori1) 
	};

	Vec3 centre2 = CC2->getWorldCentre(TC2);
	Vec3 halfExtents2 = CC2->getWorldHalfExtents(TC2);
	Quaternion ori2 = CC2->getWorldOrientation(TC2);
	// rotating the orienation around x,y,z axis in order to get local coords
	Vec3 axes2[3] = {
		QMath::rotate(Vec3(1.0f, 0.0f, 0.0f), ori2),
		QMath::rotate(Vec3(0.0f, 1.0f, 0.0f), ori2),
		QMath::rotate(Vec3(0.0f, 0.0f, 1.0f), ori2)
	};

	// Compute translation vector t
	Vec3 t = centre2 - centre1;
	
	float minPen = FLT_MAX;
	Vec3 seperationAxis;

	// lambda function to help with seperating axis tests
	// was originally going to make this into a function, something similar to the IntersectRayAABB function
	// but unlike that function, this is only used like twice, while the other one is used like 4-5 times, for both collision detection and raycasting
	// while this function is only used once here for obb-obb detection and once for aabb-obb detection, 
	// so no point in refactoring, might as well copy and paste, however if I ever need a test axis function for anything else then I'll convert it
	auto testAxis = [&](const Vec3& axis) -> bool {
		// Bring translation into a’s coordinate frame
		float axisLengthSqr = VMath::dot(axis, axis);
		// Compute common subexpressions. Add in an epsilon term to
		// counteract arithmetic errors when two edges are parallel and
		// their cross product is (near) null (see text for details)
		if (axisLengthSqr < VERY_SMALL) return true;
		Vec3 normAxis = axis / sqrt(axisLengthSqr);

		// checking for any intersections by projecting the obbs onto the axis
		float t1 = halfExtents1.x * fabs(VMath::dot(axes1[0], normAxis)) +
			halfExtents1.y * fabs(VMath::dot(axes1[1], normAxis)) +
			halfExtents1.z * fabs(VMath::dot(axes1[2], normAxis));

		float t2 = halfExtents2.x * fabs(VMath::dot(axes2[0], normAxis)) +
			halfExtents2.y * fabs(VMath::dot(axes2[1], normAxis)) +
			halfExtents2.z * fabs(VMath::dot(axes2[2], normAxis));

		float proCentre = fabs(VMath::dot(t, normAxis));

		if (proCentre > t1 + t2) return false;

		float penetration = (t1 + t2) - proCentre;
		if (penetration < minPen) {
			minPen = penetration;
			seperationAxis = normAxis;
			if (VMath::dot(t, seperationAxis) > 0) {
				// fliping the sign
				seperationAxis = seperationAxis * -1.0f;
			}
		}

		return true;
	};

	// 15 separating axis tests needed to determine OBB-OBB intersection
	
	for (int i = 0; i < 3; i++) {
		if (!testAxis(axes1[i])) return false;
		if (!testAxis(axes2[i])) return false;
	}

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (!testAxis(VMath::cross(axes1[i], axes2[j]))) return false;
		}
	}

	// Game Physics Engine Development Ian Millington 13.4.2 Colliding Two Boxes
	data.isColliding = true;
	data.contactNormal = seperationAxis;
	data.contactPoint = centre1 + seperationAxis * (minPen * 0.5f);
	data.penetration = minPen;

	// Since no separating axis is found, the OBBs must be intersecting
	return true;
}

bool CollisionSystem::SphereCapsuleDiscrete(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data)
{
	return false;
}

bool CollisionSystem::SphereCapsuleContinuous(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data)
{
	return false;
}

bool CollisionSystem::SphereAABBDiscrete(Ref<Actor> s, Ref<Actor> aabb, CollisionData& data)
{
	// Real-Time Collision Detection Ericson 5.2.5 Testing Sphere Against AABB

	Ref<TransformComponent> sTC = s->GetComponent<TransformComponent>();
	Ref<TransformComponent> abTC = aabb->GetComponent<TransformComponent>();

	Ref<CollisionComponent> sCC = s->GetComponent<CollisionComponent>();
	Ref<CollisionComponent> abCC = aabb->GetComponent<CollisionComponent>();

	Vec3 minAABB = abCC->getWorldCentre(abTC) - abCC->getWorldHalfExtents(abTC);
	Vec3 maxAABB = abCC->getWorldCentre(abTC) + abCC->getWorldHalfExtents(abTC);

	// Find point p on AABB closest to sphere center
	Vec3 closestPoint = ClosestPtPointAABB(sCC->getWorldCentre(sTC), minAABB, maxAABB);

	// Sphere and AABB intersect if the (squared) distance from sphere
	// center to point p is less than the (squared) sphere radius
	Vec3 d = sCC->getWorldCentre(sTC) - closestPoint;
	float dist2 = VMath::dot(d, d);
	if (dist2 <= (sCC->getWorldRadius(sTC) * sCC->getWorldRadius(sTC))) {
		// Collision data
		// 13.3.4 Colliding a Box and a Sphere Ian Millington
		data.isColliding = true;
		
		// adding edge case for if the sphere centre somehow gets inside the aabb
		if (sqrt(dist2) < VERY_SMALL) {
			Vec3 min = sCC->getWorldCentre(sTC) - minAABB;
			Vec3 max = maxAABB - sCC->getWorldCentre(sTC);

			float minD = min.x;
			data.contactNormal = Vec3(-1.0f, 0.0f, 0.0f);

			// if sphere is inside the aabb, have to find the closest face and push it out
			if (max.x < minD) { minD = max.x; data.contactNormal = Vec3(1.0f, 0.0f, 0.0f); }
			if (min.y < minD) { minD = min.y; data.contactNormal = Vec3(0.0f, -1.0f, 0.0f); }
			if (max.y < minD) { minD = max.y; data.contactNormal = Vec3(0.0f, 1.0f, 0.0f); }
			if (min.z < minD) { minD = min.z; data.contactNormal = Vec3(0.0f, 0.0f, -1.0f); }
			if (max.z < minD) { minD = max.z; data.contactNormal = Vec3(0.0f, 0.0f, 1.0f); }

			data.penetration = sCC->getWorldRadius(sTC) + minD;
			data.contactPoint = sCC->getWorldCentre(sTC) - data.contactNormal * sCC->getWorldRadius(sTC);
		} 
		else {
			data.contactNormal = VMath::normalize(d);
			data.contactPoint = closestPoint;
			data.penetration = sCC->getWorldRadius(sTC) - VMath::mag(d);
		}

		return true;
	}
	
	return false;
}

bool CollisionSystem::SphereAABBContinuous(Ref<Actor> s, Ref<Actor> aabb, CollisionData& data)
{
	// Real-Time Collision Detection Ericson 5.5.7 Intersecting Moving Sphere Against AABB

	Ref<TransformComponent> TC1 = s->GetComponent<TransformComponent>();
	Ref<TransformComponent> TC2 = aabb->GetComponent<TransformComponent>();

	Ref<PhysicsComponent> PC1 = s->GetComponent<PhysicsComponent>();
	Ref<PhysicsComponent> PC2 = aabb->GetComponent<PhysicsComponent>();

	Ref<CollisionComponent> CC1 = s->GetComponent<CollisionComponent>();
	Ref<CollisionComponent> CC2 = aabb->GetComponent<CollisionComponent>();

	Vec3 sCentre = CC1->getWorldCentre(TC1);
	float sRadius = CC1->getWorldRadius(TC1);
	Vec3 abCentre = CC2->getWorldCentre(TC2);
	Vec3 abHalfExtents = CC2->getWorldHalfExtents(TC2);

	// getting previous position 
	Vec3 previousPosA = sCentre - (PC1->getVel() * deltaTime); // (displacement) d=vt

	// Subtract movement of s1 from both s0 and s1, making s1 stationary
	Vec3 relVel = PC1->getVel() - PC2->getVel();

	// divide by zero check (inital velocity will start at 0, also works as a fallback)
	if (VMath::mag(relVel) < VERY_SMALL) {
		return SphereAABBDiscrete(s, aabb, data);
	}

	// Compute the AABB resulting from expanding b by sphere radius r
	Vec3 abMin = (abCentre - abHalfExtents);
	Vec3 abMax = (abCentre + abHalfExtents);

	Vec3 expandedMin = abMin - Vec3(sRadius, sRadius, sRadius);
	Vec3 expandedMax = abMax + Vec3(sRadius, sRadius, sRadius);

	// Intersect ray against expanded AABB e. Exit with no intersection if ray
	// misses e, else get intersection point p and time t as result
	RayAABBIntersection intersection = IntersectRayAABB(previousPosA, relVel, expandedMin, expandedMax, deltaTime);
	
	if (!intersection.didIntersect) return false;

	// If t is negative, ray started inside sphere so clamp t to zero (fallback to discrete)
	if (intersection.tMin < VERY_SMALL) {
		return SphereAABBDiscrete(s, aabb, data);
	}

	if (intersection.tMin > deltaTime) return false;

	data.isColliding = true;
	data.timeOfImpact = intersection.tMin / deltaTime;

	// getting what the positions are at the time of the impact
	Vec3 spherePosAtImpact = previousPosA + relVel * intersection.tMin;

	// Find point p on AABB closest to sphere center
	Vec3 closestPoint = ClosestPtPointAABB(spherePosAtImpact, abMin, abMax);

	// Sphere and AABB intersect if the (squared) distance from sphere
	// center to point p is less than the (squared) sphere radius
	Vec3 d = spherePosAtImpact - closestPoint;
	float dist = VMath::mag(d);
	
	// edge case
	if (dist < VERY_SMALL) {
		data.contactNormal = Vec3(0.0f, 0.0f, 0.0f);
		if (intersection.hitAxis == 0) {
			data.contactNormal.x = intersection.hitMaxFace ? 1.0f : -1.0f;
		}
		else if (intersection.hitAxis == 1) {
			data.contactNormal.y = intersection.hitMaxFace ? 1.0f : -1.0f;
		}
		else {
			data.contactNormal.z = intersection.hitMaxFace ? 1.0f : -1.0f;
		}
	}
	else {
		data.contactNormal = VMath::normalize(d); // d / dist;
	}

	data.contactPoint = closestPoint;
	data.penetration = 0.0f;
	data.relVel = relVel;

	return true;
}

bool CollisionSystem::SphereOBBDiscrete(Ref<Actor> s, Ref<Actor> obb, CollisionData& data)
{
	// 5.2.6 Testing Sphere Against OBB Real-Time Collision Detection

	Ref<TransformComponent> sTC = s->GetComponent<TransformComponent>();
	Ref<TransformComponent> obbTC = obb->GetComponent<TransformComponent>();

	Ref<CollisionComponent> sCC = s->GetComponent<CollisionComponent>();
	Ref<CollisionComponent> obbCC = obb->GetComponent<CollisionComponent>();

	Vec3 sCentre = sCC->getWorldCentre(sTC);
	float sRadius = sCC->getWorldRadius(sTC);

	Vec3 obbCentre = obbCC->getWorldCentre(obbTC);
	Vec3 obbHalfExtents = obbCC->getWorldHalfExtents(obbTC);
	Quaternion obbOri = obbCC->getWorldOrientation(obbTC);
	
	// setting up variables
	Vec3 v = sCentre - obbCentre;
	
	// rotating the orienation around x,y,z axis in order to get local coords
	Vec3 obblocalCoords[3] = {
		QMath::rotate(Vec3(1.0f, 0.0f, 0.0f), obbOri),
		QMath::rotate(Vec3(0.0f, 1.0f, 0.0f), obbOri),
		QMath::rotate(Vec3(0.0f, 0.0f, 1.0f), obbOri)
	};

	Vec3 sLocal(VMath::dot(v, obblocalCoords[0]),
		VMath::dot(v, obblocalCoords[1]),
		VMath::dot(v, obblocalCoords[2]));

	// Ericson has a closestpointOBB function
	// but its basically the same just making the halfextents negative
	// so reusing aabb function for closest point
	Vec3 closestPoint = ClosestPtPointAABB(sLocal, -obbHalfExtents, obbHalfExtents);
	Vec3 wClosestPoint = obbCentre + obblocalCoords[0] * closestPoint.x + obblocalCoords[1] * closestPoint.y + obblocalCoords[2] * closestPoint.z;

	// Sphere and OBB intersect if the (squared) distance from sphere
	// center to point p is less than the (squared) sphere radius
	Vec3 d = sCentre - wClosestPoint;
	float dist2 = VMath::dot(d, d);
	if (dist2 <= (sRadius * sRadius)) {
		// Collision data
		// 13.3.4 Colliding a Box and a Sphere Ian Millington
		data.isColliding = true;

		// adding edge case for if the sphere centre somehow gets inside the obb
		if (sqrt(dist2) < VERY_SMALL) {
			Vec3 min = sLocal + obbHalfExtents;
			Vec3 max = obbHalfExtents - sLocal;

			float minD = min.x;
			Vec3 localNorm = Vec3(-1.0f, 0.0f, 0.0f);

			// if sphere is inside the aabb, have to find the closest face and push it out
			if (max.x < minD) { minD = max.x; localNorm = Vec3(1.0f, 0.0f, 0.0f); }
			if (min.y < minD) { minD = min.y; localNorm = Vec3(0.0f, -1.0f, 0.0f); }
			if (max.y < minD) { minD = max.y; localNorm = Vec3(0.0f, 1.0f, 0.0f); }
			if (min.z < minD) { minD = min.z; localNorm = Vec3(0.0f, 0.0f, -1.0f); }
			if (max.z < minD) { minD = max.z; localNorm = Vec3(0.0f, 0.0f, 1.0f); }

			data.contactNormal = obblocalCoords[0] * localNorm.x + obblocalCoords[1] * localNorm.y + obblocalCoords[2] * localNorm.z;
			data.penetration = sRadius + minD;
			data.contactPoint = sCentre - data.contactNormal * sRadius;
		}
		else {
			data.contactNormal = VMath::normalize(d);
			data.contactPoint = wClosestPoint;
			data.penetration = sRadius - VMath::mag(d);
		}

		return true;
	}

	return false;
}

bool CollisionSystem::SphereOBBContinuous(Ref<Actor> s, Ref<Actor> obb, CollisionData& data)
{
	// Real-Time Collision Detection Ericson 5.5.7 Intersecting Moving Sphere Against AABB

	Ref<TransformComponent> TC1 = s->GetComponent<TransformComponent>();
	Ref<TransformComponent> TC2 = obb->GetComponent<TransformComponent>();

	Ref<PhysicsComponent> PC1 = s->GetComponent<PhysicsComponent>();
	Ref<PhysicsComponent> PC2 = obb->GetComponent<PhysicsComponent>();

	Ref<CollisionComponent> CC1 = s->GetComponent<CollisionComponent>();
	Ref<CollisionComponent> CC2 = obb->GetComponent<CollisionComponent>();

	Vec3 sCentre = CC1->getWorldCentre(TC1);
	float sRadius = CC1->getWorldRadius(TC1);

	Vec3 obCentre = CC2->getWorldCentre(TC2);
	Vec3 obHalfExtents = CC2->getWorldHalfExtents(TC2);
	Quaternion obbOri = CC2->getWorldOrientation(TC2);

	// getting previous position 
	Vec3 previousPosA = sCentre - (PC1->getVel() * deltaTime); // (displacement) d=vt

	// Subtract movement of s1 from both s0 and s1, making s1 stationary
	Vec3 relVel = PC1->getVel() - PC2->getVel();

	// divide by zero check (inital velocity will start at 0, also works as a fallback)
	if (VMath::mag(relVel) < VERY_SMALL) {
		return SphereOBBDiscrete(s, obb, data);
	}

	// rotating the orienation around x,y,z axis in order to get local coords
	Vec3 obblocalCoords[3] = {
		QMath::rotate(Vec3(1.0f, 0.0f, 0.0f), obbOri),
		QMath::rotate(Vec3(0.0f, 1.0f, 0.0f), obbOri),
		QMath::rotate(Vec3(0.0f, 0.0f, 1.0f), obbOri)
	};

	// previous pos local coords
	Vec3 v = previousPosA - obCentre;
	Vec3 prevALocal(VMath::dot(v, obblocalCoords[0]),
		VMath::dot(v, obblocalCoords[1]),
		VMath::dot(v, obblocalCoords[2]));

	Vec3 relVelLocal(VMath::dot(relVel, obblocalCoords[0]),
		VMath::dot(relVel, obblocalCoords[1]),
		VMath::dot(relVel, obblocalCoords[2]));

	// Compute the AABB resulting from expanding b by sphere radius r
	Vec3 expandedMin = -obHalfExtents - Vec3(sRadius, sRadius, sRadius);
	Vec3 expandedMax = obHalfExtents + Vec3(sRadius, sRadius, sRadius);

	// Intersect ray against expanded AABB e. Exit with no intersection if ray
	// misses e, else get intersection point p and time t as result
	RayAABBIntersection intersection = IntersectRayAABB(prevALocal, relVelLocal, expandedMin, expandedMax, deltaTime);

	if (!intersection.didIntersect) return false;

	// If t is negative, ray started inside sphere so clamp t to zero (fallback to discrete)
	if (intersection.tMin < VERY_SMALL) {
		return SphereOBBDiscrete(s, obb, data);
	}

	if (intersection.tMin > deltaTime) return false;

	data.isColliding = true;
	data.timeOfImpact = intersection.tMin / deltaTime;

	// getting what the positions are at the time of the impact
	Vec3 spherePosAtImpact = prevALocal + relVelLocal * intersection.tMin;

	// Find point p on AABB closest to sphere center
	Vec3 closestPoint = ClosestPtPointAABB(spherePosAtImpact, -obHalfExtents, obHalfExtents);

	// Sphere and AABB intersect if the (squared) distance from sphere
	// center to point p is less than the (squared) sphere radius
	Vec3 d = spherePosAtImpact - closestPoint;
	float dist = VMath::mag(d);

	// edge case
	Vec3 normLocal;
	if (dist < VERY_SMALL) {
		normLocal = Vec3(0.0f, 0.0f, 0.0f);
		if (intersection.hitAxis == 0) {
			normLocal.x = intersection.hitMaxFace ? 1.0f : -1.0f;
		}
		else if (intersection.hitAxis == 1) {
			normLocal.y = intersection.hitMaxFace ? 1.0f : -1.0f;
		}
		else {
			normLocal.z = intersection.hitMaxFace ? 1.0f : -1.0f;
		}
	}
	else {
		normLocal = VMath::normalize(d); // d / dist;
	}

	data.contactNormal = obblocalCoords[0] * normLocal.x + obblocalCoords[1] * normLocal.y + obblocalCoords[2] * normLocal.z;
	data.contactPoint = obCentre + obblocalCoords[0] * closestPoint.x + obblocalCoords[1] * closestPoint.y + obblocalCoords[2] * closestPoint.z;
	data.penetration = 0.0f;
	data.relVel = relVel;

	return true;
}

bool CollisionSystem::CapsuleAABBDiscrete(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data)
{
	return false;
}

bool CollisionSystem::CapsuleAABBContinuous(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data)
{
	return false;
}

bool CollisionSystem::CapsuleOBBDiscrete(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data)
{
	return false;
}

bool CollisionSystem::CapsuleOBBContinuous(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data)
{
	return false;
}

bool CollisionSystem::AABBOBBDiscrete(Ref<Actor> ab, Ref<Actor> ob, CollisionData& data)
{
	// litteraly the extact same as the obb-obb test, just with the aabb having default axes

	Ref<TransformComponent> TC1 = ab->GetComponent<TransformComponent>();
	Ref<TransformComponent> TC2 = ob->GetComponent<TransformComponent>();

	Ref<CollisionComponent> CC1 = ab->GetComponent<CollisionComponent>();
	Ref<CollisionComponent> CC2 = ob->GetComponent<CollisionComponent>();

	// setting up variables
	Vec3 centre1 = CC1->getWorldCentre(TC1);
	Vec3 halfExtents1 = CC1->getWorldHalfExtents(TC1);
	// aabb just giving it default axes
	Vec3 axes1[3] = {
		Vec3(1.0f, 0.0f, 0.0f),
		Vec3(0.0f, 1.0f, 0.0f),
		Vec3(0.0f, 0.0f, 1.0f)
	};

	Vec3 centre2 = CC2->getWorldCentre(TC2);
	Vec3 halfExtents2 = CC2->getWorldHalfExtents(TC2);
	Quaternion ori2 = CC2->getWorldOrientation(TC2);
	// rotating the orienation around x,y,z axis in order to get local coords
	Vec3 axes2[3] = {
		QMath::rotate(Vec3(1.0f, 0.0f, 0.0f), ori2),
		QMath::rotate(Vec3(0.0f, 1.0f, 0.0f), ori2),
		QMath::rotate(Vec3(0.0f, 0.0f, 1.0f), ori2)
	};

	// Compute translation vector t
	Vec3 t = centre2 - centre1;

	float minPen = FLT_MAX;
	Vec3 seperationAxis;

	// lambda function to help with seperating axis tests
	// was originally going to make this into a function, something similar to the IntersectRayAABB function
	// but unlike that function, this is only used like twice, while the other one is used like 4-5 times, for both collision detection and raycasting
	// while this function is only used once here for obb-obb detection and once for aabb-obb detection, 
	// so no point in refactoring, might as well copy and paste, however if I ever need a test axis function for anything else then I'll convert it
	auto testAxis = [&](const Vec3& axis) -> bool {
		// Bring translation into a’s coordinate frame
		float axisLengthSqr = VMath::dot(axis, axis);
		// Compute common subexpressions. Add in an epsilon term to
		// counteract arithmetic errors when two edges are parallel and
		// their cross product is (near) null (see text for details)
		if (axisLengthSqr < VERY_SMALL) return true;
		Vec3 normAxis = axis / sqrt(axisLengthSqr);

		// checking for any intersections by projecting the obbs onto the axis
		float t1 = halfExtents1.x * fabs(VMath::dot(axes1[0], normAxis)) +
			halfExtents1.y * fabs(VMath::dot(axes1[1], normAxis)) +
			halfExtents1.z * fabs(VMath::dot(axes1[2], normAxis));

		float t2 = halfExtents2.x * fabs(VMath::dot(axes2[0], normAxis)) +
			halfExtents2.y * fabs(VMath::dot(axes2[1], normAxis)) +
			halfExtents2.z * fabs(VMath::dot(axes2[2], normAxis));

		float proCentre = fabs(VMath::dot(t, normAxis));

		if (proCentre > t1 + t2) return false;

		float penetration = (t1 + t2) - proCentre;
		if (penetration < minPen) {
			minPen = penetration;
			seperationAxis = normAxis;
			if (VMath::dot(t, seperationAxis) > 0) {
				// fliping the sign
				seperationAxis = seperationAxis * -1.0f;
			}
		}

		return true;
		};

	// 15 separating axis tests needed to determine OBB-OBB intersection

	for (int i = 0; i < 3; i++) {
		if (!testAxis(axes1[i])) return false;
		if (!testAxis(axes2[i])) return false;
	}

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (!testAxis(VMath::cross(axes1[i], axes2[j]))) return false;
		}
	}

	// Game Physics Engine Development Ian Millington 13.4.2 Colliding Two Boxes
	data.isColliding = true;
	data.contactNormal = seperationAxis;
	data.contactPoint = centre1 + seperationAxis * (minPen * 0.5f);
	data.penetration = minPen;

	// Since no separating axis is found, the OBBs must be intersecting
	return true;
}

/*

Ref<Actor> CollisionSystem::PhysicsRaycast(Vec3 start, Vec3 end) {

	for (Ref<Actor> obj : collidingActors) {
		//std::cout << obj->getActorName();
		Vec3 dir = end - start;
		
		Ref<TransformComponent> transform = obj->GetComponent<TransformComponent>();
		Ref<CollisionComponent> collider = obj->GetComponent<CollisionComponent>();

		if (collider && transform && collider->colliderType == ColliderType::SPHERE) {

			//make a sphere clickable construct
			Sphere s1;

			//Get position is unreliable due to parenting altering the transform
			//s1.centre = transform->GetPosition();
			s1.r = collider->radius;

			Matrix4 modelMatrix = obj->GetModelMatrix();

			// Apply transforms before checking, the origin should be 0,0,0 and must be the centre
			s1.centre = Vec3(modelMatrix * Vec4(Vec3(0.0f,0.0f,0.0f), 1.0f));


			Vec3 toPoint = s1.centre - start;

			float t = VMath::dot(toPoint, dir) / VMath::dot(dir, dir);  // projection scalar

			Vec3 closestPoint = start + dir * t;

			float distance = VMath::distance(s1.centre, closestPoint);
			std::cout << obj->getActorName() << std::endl;
			std::cout <<distance << " < " << s1.r << std::endl;
			if (distance < s1.r) { return obj; }

			
		}
	}
	return nullptr;
}


*/

// Computes closest points C1 and C2 of S1(s)=P1+s*(Q1-P1) and
// S2(t)=P2+t*(Q2-P2), returning s and t. Function result is squared
// distance between between S1(s) and S2(t
float CollisionSystem::ClosestPtSegmentSegment(Vec3 p1, Vec3 q1, Vec3 p2, Vec3 q2,
											   float& s, float& t, Vec3& c1, Vec3& c2)
{
	// 5.1.9 Closest Points of Two Line Segments

	Vec3 d1 = q1 - p1; // Direction vector of segment S1
	Vec3 d2 = q2 - p2; // Direction vector of segment S2
	Vec3 r = p1 - p2;
	float a = VMath::dot(d1, d1); // Squared length of segment S1, always nonnegative
	float e = VMath::dot(d2, d2); // Squared length of segment S2, always nonnegative
	float f = VMath::dot(d2, r);
	// Check if either or both segments degenerate into points
	if (a <= VERY_SMALL && e <= VERY_SMALL) {
		// Both segments degenerate into points
		s = t = 0.0f;
		c1 = p1;
		c2 = p2; 
		return VMath::dot(c1 - c2, c1 - c2);
	}
	if (a <= VERY_SMALL) {
		// First segment degenerates into a point
		s = 0.0f;
		t = f / e; // s = 0 => t = (b*s + f) / e = f / e
		t = std::max(0.0f, std::min(1.0f, t));
	}
	else {
		float c = VMath::dot(d1, r);
		if (e <= VERY_SMALL) {
			// Second segment degenerates into a point
			t = 0.0f;
			s = std::max(0.0f, std::min(1.0f, -c / a)); // t = 0 => s = (b*t - c) / a = -c / a
		}
		else {
			// The general nondegenerate case starts here
			float b = VMath::dot(d1, d2);
			float denom = a * e - b * b; // Always nonnegative
			// If segments not parallel, compute closest point on L1 to L2 and
			// clamp to segment S1. Else pick arbitrary s (here 0)
			if (denom != 0.0f) {
				s = std::max(0.0f, std::min(1.0f, (b * f - c * e) / denom));
			}
			else s = 0.0f;
			// Compute point on L2 closest to S1(s) using
			// t = Dot((P1 + D1*s) - P2,D2) / Dot(D2,D2) = (b*s + f) / e
			t = (b * s + f) / e;// If t in [0,1] done. Else clamp t, recompute s for the new value
			// of t using s = Dot((P2 + D2*t) - P1,D1) / Dot(D1,D1)= (t*b - c) / a
			// and clamp s to [0, 1]
			if (t < 0.0f) {
				t = 0.0f;
				s = std::max(0.0f, std::min(1.0f, -c / a));
			}
			else if (t > 1.0f) {
				t = 1.0f;
				s = std::max(0.0f, std::min(1.0f, (b - c) / a));
			}
		}
	}

	c1 = p1 + d1 * s;
	c2 = p2 + d2 * t;
	return VMath::dot(c1 - c2, c1 - c2);
}

// Given point p, return the point q on or in AABB b that is closest to p
Vec3 CollisionSystem::ClosestPtPointAABB(const Vec3& p, const Vec3& aabbMin, const Vec3& aabbMax)
{
	// Real-Time Collision Detection Ericson 5.1.3 Closest Point on AABB to Point

	// For each coordinate axis, if the point coordinate value is
	// outside box, clamp it to the box, else keep it as is
	Vec3 closestPoint;
	closestPoint.x = std::max(aabbMin.x, std::min(p.x, aabbMax.x)); // v = max(v, b.min[i])
	closestPoint.y = std::max(aabbMin.y, std::min(p.y, aabbMax.y)); 
	closestPoint.z = std::max(aabbMin.z, std::min(p.z, aabbMax.z)); 
	return closestPoint;
}

// Intersect ray R(t) = p + t*d against AABB a. When intersecting,
// return intersection distance tmin and point q of intersection
CollisionSystem::RayAABBIntersection CollisionSystem::IntersectRayAABB(const Vec3& rayOri_, const Vec3& rayDir_, const Vec3& boxMin_, const Vec3& boxMax_, float maxDistance)
{
	// Real-Time Collision Detection Ericson 5.3.3 Intersecting Ray or Segment Against Box

	RayAABBIntersection result;

	float tmin = 0.0f; // set to -FLT_MAX to get first hit on line
	float tmax = maxDistance; // set to max distance ray can travel (for segment)

	int hitAxis = -1;
	bool hitMaxFace = false;

	// For all three slabs
	for (int i = 0; i < 3; i++) {
		// vec3 extraction
		float rayOrigin = (i == 0) ? rayOri_.x : (i == 1) ? rayOri_.y : rayOri_.z; // p
		float rayDir = (i == 0) ? rayDir_.x : (i == 1) ? rayDir_.y : rayDir_.z; // d
		float boxMin = (i == 0) ? boxMin_.x : (i == 1) ? boxMin_.y : boxMin_.z;
		float boxMax = (i == 0) ? boxMax_.x : (i == 1) ? boxMax_.y : boxMax_.z;

		if (fabs(rayDir) < VERY_SMALL) {
			// Ray is parallel to slab. No hit if origin not within slab
			if (rayOrigin < boxMin || rayOrigin > boxMax) {
				result.didIntersect = false;
				return result;
			}
		}
		else {
			// Compute intersection t value of ray with near and far plane of slab
			float ood = 1.0f / rayDir;
			float t1 = (boxMin - rayOrigin) * ood;
			float t2 = (boxMax - rayOrigin) * ood;
			// Make t1 be intersection with near plane, t2 with far plane
			bool swapped = false;
			if (t1 > t2) {
				std::swap(t1, t2);
				swapped = true;
			}
			// Compute the intersection of slab intersection intervals
			if (t1 > tmin) {
				tmin = t1;
				hitAxis = i;
				hitMaxFace = swapped;
			}
			if (t2 > tmax) tmax = t2;
			// Exit with no collision as soon as slab intersection becomes empty
			if (tmin > tmax) {
				result.didIntersect = false;
				return result;
			}
		}
	}
	
	result.didIntersect = true;
	result.tMin = tmin;
	result.tMax = tmax;
	result.hitAxis = hitAxis;
	result.hitMaxFace = hitMaxFace;

	return result;
}
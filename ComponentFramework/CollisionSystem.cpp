#include "CollisionSystem.h"

Ref<Actor> CollisionSystem::PhysicsRaycast(Vec3 start, Vec3 end) {

	for (Ref<Actor> obj : collidingActors) {
		//std::cout << obj->getActorName();
		Vec3 dir = end - start;
		
		Ref<TransformComponent> transform = obj->GetComponent<TransformComponent>();
		Ref<CollisionComponent> collider = obj->GetComponent<CollisionComponent>();

		if (collider && transform && collider->colliderType == ColliderType::SPHERE) {

			//make a sphere clickable construct
			Sphere s1;
			s1.center = transform->GetPosition();
			s1.r = collider->radius;


			Vec3 toPoint = s1.center - start;

			float t = VMath::dot(toPoint, dir) / VMath::dot(dir, dir);  // projection scalar

			Vec3 closestPoint = start + dir * t;

			float distance = VMath::distance(s1.center, closestPoint);


			//std::cout << distance << std::endl;
			//std::cout << s1.r << std::endl;

			if (distance < s1.r) { std::cout << "clicked" << std::endl; return obj; }

			
		}
	}
	return nullptr;
}


bool CollisionSystem::CollisionDetection(const Sphere& s1, const Sphere& s2) const
{
	// from semester 2 but also from the Real-Time Collision Detection book 4.5.1 "Sphere-swept Volume Intersection"

	// Step 1
	// Find the distance between the two bodies
	float distance = VMath::distance(s1.center, s2.center);
	// Step 2
	// Compare the distance to the sum of the radii
	// If distance <= r1 + r2, then result = true
	float radius = s1.r + s2.r;
	return distance <= radius * radius;
}

bool CollisionSystem::CollisionDetection(const AABB& bb1, const AABB& bb2 ) const {
	
	// from Real-Time Collision Detection 4.2.1 "AABB-AABB Intersection" 

	if (abs(bb1.center.x - bb2.center.x) > (bb1.halfExtents.x + bb2.halfExtents.x)) return false;
	if (abs(bb1.center.y - bb2.center.y) > (bb1.halfExtents.y + bb2.halfExtents.y)) return false;
	if (abs(bb1.center.z - bb2.center.z) > (bb1.halfExtents.z + bb2.halfExtents.z)) return false;

	return true;
}

bool CollisionSystem::CollisionDetection(const Sphere s1, const Plane p1) const
{
	// from Real-Time Collision Detection 5.2.2 "Testing Sphere Against Plane"

	// For a normalized plane (|p.n| = 1), evaluating the plane equation
	Plane p1Norm = PMath::normalize(p1);
	
	// for a point gives the signed distance of the point to the plane
	float dist = PMath::distance(s1.center, p1Norm);
	
	// If sphere center within +/-radius from plane, plane intersects sphere
	return abs(dist) <= s1.r;
}

void CollisionSystem::SphereSphereCollisionResponse(Ref<Actor> s1, Ref<Actor> s2)
{
	// get the physics and transform components from the actors

	Ref<PhysicsComponent> PC1 = s1->GetComponent<PhysicsComponent>();
	Ref<PhysicsComponent> PC2 = s2->GetComponent<PhysicsComponent>();

	Ref<TransformComponent> TC1 = s1->GetComponent<TransformComponent>();
	Ref<TransformComponent> TC2 = s2->GetComponent<TransformComponent>();

	// this code is from semester 2 when umer taught spheresphere collision response, I just modified it a bit

	// Step 1, find the normal vector (vector from body2 to body1)
	Vec3 normal = TC2->GetPosition() - (VMath::normalize(TC1->GetPosition()));
	
	// Step 2, find the relative velocity
	Vec3 relVel = PC1->getVel() - PC2->getVel();

	// if relvel = 0, 

	// Step 3, find relative velocity along the normal
	float relVelAlongNormal = VMath::dot(relVel, normal);
	// Step 4, ignore everything if relVelAlongNormal < 0
	// We don't care about collisions if the objects
	// are moving away from each other
	if (relVelAlongNormal < 0) {
		return; // get outta here
	}
	// Step 5, calculate J using my assignment notes
	float E = 1.0f; // coefficent of resitition
	float J = (-(1.0f + E) * relVelAlongNormal) / ((1.0f / PC1->getMass()) + (1.0f / PC2->getMass()));
	// Step 6, update the velocities
	PC1->setVel(PC1->getVel() + (J / PC1->getMass()) * normal);
	PC2->setVel(PC2->getVel() - (J / PC2->getMass()) * normal);

}

void CollisionSystem::AABBAABBCollisionResponse(Ref<Actor> bb1, Ref<Actor> bb2)
{
	std::cout << "COLLISION DETECTED" << std::endl;
}

void CollisionSystem::SpherePlaneCollisionResponse(Ref<Actor> s1, Ref<Actor> p1)
{
	std::cout << "COLLISION DETECTED" << std::endl;
}

void CollisionSystem::Update(const float deltaTime)
{
	// loop through all colliding actors, two times cause two objects colliding
	for (size_t i = 0; i < collidingActors.size(); i++) {
		for (size_t j = i + 1; j < collidingActors.size(); j++) {

			// get the collision component from the colliding actors
			Ref<CollisionComponent> CC1 = collidingActors[i]->GetComponent<CollisionComponent>();
			Ref<CollisionComponent> CC2 = collidingActors[j]->GetComponent<CollisionComponent>();

			// if both collider types are spheres...
			if (CC1->colliderType == ColliderType::SPHERE &&
				CC2->colliderType == ColliderType::SPHERE) {

				// get the transform components from the colliding actors
				Ref<TransformComponent> TC1 = collidingActors[i]->GetComponent<TransformComponent>();
				Ref<TransformComponent> TC2 = collidingActors[j]->GetComponent<TransformComponent>();

				// this is setting up all the info for the spheres
				Sphere s1, s2;

				s1.center = TC1->GetPosition();
				s1.r = CC1->radius;

				s2.center = TC2->GetPosition();
				s2.r = CC2->radius;

				// if collisison was detected between the two spheres, do response
				if (CollisionDetection(s1, s2)) {
					SphereSphereCollisionResponse(collidingActors[i], collidingActors[j]);
				}

			}

			// if both collider types are AABBs...
			if (CC1->colliderType == ColliderType::AABB &&
				CC2->colliderType == ColliderType::AABB) {

				// get the transform components from the colliding actors
				Ref<TransformComponent> TC1 = collidingActors[i]->GetComponent<TransformComponent>();
				Ref<TransformComponent> TC2 = collidingActors[j]->GetComponent<TransformComponent>();

				// this is setting up all the info for the AABBs
				AABB bb1, bb2;

				bb1.center = TC1->GetPosition();
				bb1.halfExtents = CC1->halfExtents;

				bb2.center = TC1->GetPosition();
				bb2.halfExtents = CC2->halfExtents;

				// if collision was detected between the two AABBs, do response
				if (CollisionDetection(bb1, bb2)) {
					AABBAABBCollisionResponse(collidingActors[i], collidingActors[j]);
				}
				// debug check to see if collision is being detected
				/*else {
					std::cout << "NO COLLISION DETECTED" << std::endl;
				}*/

			}

			// if theres a sphere and plane collider...
			if (CC1->colliderType == ColliderType::SPHERE &&
				CC2->colliderType == ColliderType::PLANE) {

				// get the transform components from the colliding actors
				Ref<TransformComponent> TC1 = collidingActors[i]->GetComponent<TransformComponent>();
				Ref<TransformComponent> TC2 = collidingActors[j]->GetComponent<TransformComponent>();

				// set up all the info for the sphere and the plane
				Sphere s1;
				Plane p1;

				s1.center = TC1->GetPosition();
				s1.r = CC1->radius;

				// p1

				// if collision was detected between the sphere and the plane, do response
				if (CollisionDetection(s1, p1)) {
					SpherePlaneCollisionResponse(collidingActors[i], collidingActors[j]);
				}
				// debug to check if collision is detected
				/*else {
					std::cout << "NO COLLISION DETECTED" << std::endl;
				}*/

			}

		}
	}

}


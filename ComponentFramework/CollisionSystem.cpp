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
}

bool CollisionSystem::CollisionDetection(Ref<Actor> s1, Ref<Actor> s2, CollisionData& data)
{
	Ref<CollisionComponent> CC1 = s1->GetComponent<CollisionComponent>();
	Ref<CollisionComponent> CC2 = s2->GetComponent<CollisionComponent>();
	
	if (CC1->getState() == ColliderState::Continuous || CC2->getState() == ColliderState::Continuous) {
		return SphereSphereContinuous(s1, s2, data);
	}
	else {
		return SphereSphereDiscrete(s1, s2, data);
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

	// Expand sphere s1 by the radius of s0
	float expRadi = CC2->getWorldRadius(TC2) + CC1->getWorldRadius(TC1);
	// Subtract movement of s1 from both s0 and s1, making s1 stationary
	Vec3 v = PC1->getVel() - PC2->getVel();
	
	// divide by zero check (inital velocity will start at 0, also works as a fallback)
	if (VMath::mag(v) < VERY_SMALL) {
		return SphereSphereDiscrete(s1, s2, data);
	}

	// Can now test directed segment s = s0.c + tv, v = (v0-v1)/||v0-v1|| against
	// the expanded sphere for intersection
	float vlen = VMath::mag(v) * deltaTime;
	
	Vec3 m = CC1->getWorldCentre(TC1) - CC2->getWorldCentre(TC2);
	float b = VMath::dot(m, v / vlen);
	float c = VMath::dot(m, m) - CC2->getWorldRadius(TC2) * CC2->getWorldRadius(TC2);
	// Exit if r’s origin outside s (c > 0) and r pointing away from s (b > 0)
	if (c > 0.0f && b > 0.0f) return 0;
	float discr = b * b - c;
	// A negative discriminant corresponds to ray missing sphere
	if (discr < 0.0f) return 0;
	// Ray now found to intersect sphere, compute smallest t value of intersection
	float t = -b - sqrt(discr);
	// If t is negative, ray started inside sphere so clamp t to zero
	if (t < 0.0f) t = 0.0f;

	//IntersectRaySphere(Point p, Vector d, Sphere s, float &t, Point &q)

	// IntersectRaySphere(s0.c, v / vlen, s1, t, q)

	return true;
}

void CollisionSystem::Update(float deltaTime) {
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

			

			// if both collider types are spheres...
			if (CC1->colliderType == ColliderType::Sphere &&
				CC2->colliderType == ColliderType::Sphere) {
				
				CollisionData data;

				// if collisison was detected between the two spheres, do response
				if (CollisionDetection(a1, a2, data)) {
					
					//printf("x: %f, y: %f, z: %f\n", data.contactNormal.x, data.contactNormal.y, data.contactNormal.z);

					//std::cout << data.penetration << std::endl;
					std::cout << "COLLSION DETECTED!" << std::endl;
				}
			}
		}

	}
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


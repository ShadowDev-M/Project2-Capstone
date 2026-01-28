#include "pch.h"
#include "PhysicsSystem.h"
#include "TransformComponent.h"

void PhysicsSystem::AddActor(Ref<Actor> actor_) {
	if (actor_->GetComponent<TransformComponent>().get() == nullptr) {
		Debug::Error("The Actor must have a TransformComponent - ignored ", __FILE__, __LINE__);
		return;
	}

	if (actor_->GetComponent<PhysicsComponent>().get() == nullptr) {
		Debug::Error("The Actor must have a PhysicsComponent - ignored ", __FILE__, __LINE__);
		return;
	}

	// additional check to make sure not adding duplicate actors
	if (std::find(physicsActors.begin(), physicsActors.end(), actor_) != physicsActors.end()) {
		Debug::Warning("Actor already added to PhysicsSystem", __FILE__, __LINE__);
		return;
	}

	physicsActors.push_back(actor_);
}

void PhysicsSystem::RemoveActor(Ref<Actor> actor_)
{
	// finding actor from vector and removing it
	auto it = std::find(physicsActors.begin(), physicsActors.end(), actor_);
	if (it != physicsActors.end()) {
		physicsActors.erase(it);
	}
}

void PhysicsSystem::Update(float deltaTime) 
{
	for (Ref<Actor> actor : physicsActors) {
		// getting transform and physics components
		Ref<TransformComponent> TC = actor->GetComponent<TransformComponent>();
		Ref<PhysicsComponent> PC = actor->GetComponent<PhysicsComponent>();

		// if the pc is static then it doesn't get affected by physics
		if (PC->getState() == PhysicsState::Static) continue; 

		// if its kinematic, then it only gets affected by velocity, for basic motion but not other forces like gravity, drag, etc
		if (PC->getState() == PhysicsState::Kinematic) {
			UpdatePos(actor, deltaTime);
			continue;
		}

		// rest is for dynamic pcs now
		
		Vec3 netForce;

		// applying gravity
		if (PC->useGravity == true) {
			Vec3 gravityForce(0.0f, gravity * PC->mass, 0.0f);
			
			// adding gravity to net force
			netForce += gravityForce;
		}


		// applying drag
		Vec3 dragForce = PC->vel * (-PC->drag);
		
		// adding drag to net force
		netForce += dragForce;
		
		ApplyForce(actor, netForce);

		UpdateVel(actor, deltaTime);
		UpdatePos(actor, deltaTime);

		// TODO: check with umer to make sure angular calculations are correct
		//Vec3 angularDragFroce = PC->angularVel * (-PC->angularDrag);
		//ApplyTorque(actor, angularDragFroce);

		// updating orientation/angular velocity
		UpdateOrientation(actor, deltaTime);
	}
}

void PhysicsSystem::UpdatePos(Ref<Actor> actor_, float deltaTime)
{
	Ref<TransformComponent> TC = actor_->GetComponent<TransformComponent>();
	Ref<PhysicsComponent> PC = actor_->GetComponent<PhysicsComponent>();

	Vec3 TCPos = TC->GetPosition();
	TCPos += PC->vel * deltaTime;
	TC->SetPos(TCPos.x, TCPos.y, TCPos.z);
}

void PhysicsSystem::UpdateVel(Ref<Actor> actor_, float deltaTime)
{
	Ref<PhysicsComponent> PC = actor_->GetComponent<PhysicsComponent>();

	PC->vel += PC->accel * deltaTime;
}

void PhysicsSystem::UpdateOrientation(Ref<Actor> actor_, float deltaTime)
{
	Ref<TransformComponent> TC = actor_->GetComponent<TransformComponent>();
	Ref<PhysicsComponent> PC = actor_->GetComponent<PhysicsComponent>();

	// Pull out angle from angular velocity
	float angleDegrees = VMath::mag(PC->angularVel) * deltaTime * RADIANS_TO_DEGREES;
	// if angle is zero get outta here
	if (angleDegrees < VERY_SMALL) {
		return;
	}
	// Axis of rotation
	Vec3 axis = VMath::normalize(PC->angularVel);
	// Build a quaternion
	Quaternion rotation = QMath::angleAxisRotation(angleDegrees, axis);
	// Update the orientation
	Quaternion newOri = rotation * TC->GetQuaternion();
	TC->SetOrientation(newOri);
}

void PhysicsSystem::ApplyForce(Ref<Actor> actor_, const Vec3& force)
{
	Ref<PhysicsComponent> PC = actor_->GetComponent<PhysicsComponent>();

	// if actor doesnt have a physics component or isnt dynamic then return
	if (!PC || PC->getState() != PhysicsState::Dynamic) return;

	PC->accel = force / PC->mass;
}

void PhysicsSystem::ApplyImpulse(Ref<Actor> actor_, const Vec3& impulse)
{
	Ref<PhysicsComponent> PC = actor_->GetComponent<PhysicsComponent>();

	// if actor doesnt have a physics component or isnt dynamic then return
	if (!PC || PC->getState() != PhysicsState::Dynamic) return;

	PC->vel = impulse / PC->mass;
}

void PhysicsSystem::ResetPhysics()
{
	for (Ref<Actor> actor : physicsActors) {
		Ref<PhysicsComponent> PC = actor->GetComponent<PhysicsComponent>();

		PC->setVel(Vec3(0.0f, 0.0f, 0.0f));
		PC->setAccel(Vec3(0.0f, 0.0f, 0.0f));
		PC->setAngularVel(Vec3(0.0f, 0.0f, 0.0f));
		PC->setAngularAccel(Vec3(0.0f, 0.0f, 0.0f));
	}
}

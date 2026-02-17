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
	for (const Ref<Actor>& actor : physicsActors) {
		// getting transform and physics components
		Ref<TransformComponent> TC = actor->GetComponent<TransformComponent>();
		Ref<PhysicsComponent> PC = actor->GetComponent<PhysicsComponent>();

		// if the pc is static then it doesn't get affected by physics
		if (PC->getState() == PhysicsState::Static) continue; 

		// if its kinematic, then it only gets affected by velocity, for basic motion but not other forces like gravity, drag, etc
		if (PC->getState() == PhysicsState::Kinematic) {
			UpdatePos(actor, deltaTime);
			UpdateOrientation(actor, deltaTime);
			continue;
		}

		// rest is for dynamic pcs now

		// applying gravity
		if (PC->useGravity == true) {
			PC->AddForce(Vec3(0.0f, gravity * PC->mass, 0.0f));
		}

		// adding drag 
		PC->AddForce(PC->vel * (-PC->drag));

		// linear motion
		Vec3 linearAccel = PC->forceAccumulator * PC->getInverseMass();
		PC->setAccel(linearAccel);

		// TODO: angular motion
		
		Vec3 newVel = PC->vel + linearAccel * deltaTime;
		PC->setVel(newVel);

		// for the inspector
		PC->accel = linearAccel;

		UpdatePos(actor, deltaTime);
		UpdateOrientation(actor, deltaTime);

		// clearing any accumulated forces from previous frame
		PC->ClearAccumulators();
	}
}

void PhysicsSystem::UpdateVel(Ref<Actor> actor_, float deltaTime)
{
	Ref<PhysicsComponent> PC = actor_->GetComponent<PhysicsComponent>();

	PC->vel += PC->accel * deltaTime;
}

void PhysicsSystem::UpdatePos(Ref<Actor> actor_, float deltaTime)
{
	Ref<TransformComponent> TC = actor_->GetComponent<TransformComponent>();
	Ref<PhysicsComponent> PC = actor_->GetComponent<PhysicsComponent>();

	Vec3 nVel = PC->vel;

	Vec3 TCPos = TC->GetPosition();
	TCPos += nVel * deltaTime;
	TC->SetPos(TCPos.x, TCPos.y, TCPos.z);
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
	Quaternion newOri = rotation * TC->GetOrientation();
	TC->SetOrientation(newOri);
}

void PhysicsSystem::ResetPhysics()
{
	for (Ref<Actor> actor : physicsActors) {
		Ref<PhysicsComponent> PC = actor->GetComponent<PhysicsComponent>();

		PC->setVel(Vec3(0.0f, 0.0f, 0.0f));
		PC->setAccel(Vec3(0.0f, 0.0f, 0.0f));
		PC->setAngularVel(Vec3(0.0f, 0.0f, 0.0f));
		PC->setAngularAccel(Vec3(0.0f, 0.0f, 0.0f));
		PC->ClearAccumulators();
	}
}
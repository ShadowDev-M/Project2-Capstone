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
	//for (const Ref<Actor>& actor : physicsActors) {
	//	// getting transform and physics components
	//	Ref<TransformComponent> TC = actor->GetComponent<TransformComponent>();
	//	Ref<PhysicsComponent> PC = actor->GetComponent<PhysicsComponent>();

	//	// if the pc is static then it doesn't get affected by physics
	//	if (PC->getState() == PhysicsState::Static) continue; 

	//	// if its kinematic, then it only gets affected by velocity, for basic motion but not other forces like gravity, drag, etc
	//	if (PC->getState() == PhysicsState::Kinematic) {
	//		ApplyConstraints(PC);
	//		UpdatePos(actor, deltaTime);
	//		UpdateOrientation(actor, deltaTime);
	//		continue;
	//	}

	//	// rest is for dynamic pcs now

	//	// applying gravity
	//	if (PC->useGravity == true) {
	//		PC->AddForce(Vec3(0.0f, gravity * PC->mass, 0.0f));
	//	}

	//	// applying drag
	//	Vec3 dragForce = PC->vel * (-PC->drag);
	//	
	//	// adding drag to net force
	//	PC->AddForce(PC->vel * (-PC->drag));

	//	// linear motion
	//	Vec3 linearAccel = PC->forceAccumulator * PC->getInverseMass();

	//	// TODO: angular motion
	//	
	//	PC->vel += linearAccel * deltaTime;

	//	// for the inspector
	//	PC->accel = linearAccel;

	//	ApplyConstraints(PC);

	//	UpdatePos(actor, deltaTime);
	//	UpdateOrientation(actor, deltaTime);

	//	// clearing any accumulated forces from previous frame
	//	PC->ClearAccumulators();
	//}

	for (const Ref<Actor>& actor : physicsActors) {
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
		
		//TODO
		ApplyForce(actor, netForce);
		


		UpdateVel(actor, deltaTime);
		

		UpdatePos(actor, deltaTime);
		
		// TODO: check with umer to make sure angular calculations are correct
		//Vec3 angularDragFroce = PC->angularVel * (-PC->angularDrag);
		//ApplyTorque(actor, angularDragFroce);

		// updating orientation/angular velocity
		UpdateOrientation(actor, deltaTime);
		
		ApplyForce(actor, -netForce);
		
	}
}

Vec3 PhysicsSystem::ResolveConstraintsPos(Ref<PhysicsComponent> PC, Vec3 vector_) {
	if (PC->constraints.freezePosX) {
		vector_.x = 0;
	}
	if (PC->constraints.freezePosY) {
		vector_.y = 0;
	}
	if (PC->constraints.freezePosZ) {
		vector_.z = 0;
	}

	return vector_;
}

void PhysicsSystem::ApplyForce(Ref<Actor> actor_, const Vec3& force)
{
	Ref<PhysicsComponent> PC = actor_->GetComponent<PhysicsComponent>();



	// if actor doesnt have a physics component or isnt dynamic then return
	if (!PC || PC->getState() != PhysicsState::Dynamic) return;
	
	Vec3 nForce = ResolveConstraintsPos(PC, force);

	PC->accel += nForce / PC->mass;
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
		//PC->ClearAccumulators();
	}
}

void PhysicsSystem::ApplyConstraints(Ref<PhysicsComponent> PC)
{
	if (!PC) return;

	const PhysicsConstraints& constraints = PC->constraints;

	Vec3 vel = PC->vel;
	if (constraints.freezePosX) vel.x = 0.0f;
	if (constraints.freezePosY) vel.y = 0.0f;
	if (constraints.freezePosZ) vel.z = 0.0f;
	PC->vel = vel;

	Vec3 angularVel = PC->angularVel;
	if (constraints.freezeRotX) angularVel.x = 0.0f;
	if (constraints.freezeRotY) angularVel.y = 0.0f;
	if (constraints.freezeRotZ) angularVel.z = 0.0f;
	PC->angularVel = angularVel;
}
#include "pch.h"
#include "Actor.h"
#include "TransformComponent.h"
#include "LightComponent.h"
#include "MeshComponent.h"
#include "InputManager.h"
#include "SceneGraph.h"
#include "AnimatorComponent.h"
static uint32_t idCounter = 1;

Actor::Actor(Component* parent_) :Component(parent_) { id = idCounter++; }

Actor::Actor(Component* parent_, const std::string& actorName_): Component(parent_), actorName(actorName_) { id = idCounter++; }


bool Actor::OnCreate() {
	if (isCreated) return true;
	Debug::Info("Loading assets for Actor: ", __FILE__, __LINE__);
	
	// Loops over all components
	for (auto& component : components) {

		std::string componentType = static_cast<std::string>(typeid(component.get()).name()).substr(6); //TODO: substr removes the 'class ' that gets added when getting the name from a typeid, 

		if (std::dynamic_pointer_cast<ShaderComponent>(component) ||
			std::dynamic_pointer_cast<MeshComponent>(component) ||
			std::dynamic_pointer_cast<MaterialComponent>(component) ||
			std::dynamic_pointer_cast<ScriptComponent>(component)) {
			continue;
		}

		if (component->OnCreate() == false) {
			Debug::Error("Loading assets for Actor/Components: ", __FILE__, __LINE__);
			isCreated = false;
			return isCreated;
		}
	}
	isCreated = true;
	return isCreated;
}

Actor::~Actor() {
	OnDestroy();
}

void Actor::OnDestroy() {
	if (!isCreated) return;

	Debug::Info("Deleting assets for Actor: ", __FILE__, __LINE__);
	RemoveAllComponents();
	isCreated = false;

	
}



void Actor::Update(const float deltaTime) {
	
}

void Actor::Render()const {}

bool Actor::ValidateLight()
{
	if (GetComponent<LightComponent>() && GetComponent<TransformComponent>()) {
		SceneGraph& sg = SceneGraph::getInstance();


		return true;
	}
	return false;
}

bool Actor::InitalizeLight()
{
	if (!SceneGraph::getInstance().GetLightExist(SceneGraph::getInstance().GetActor(actorName))) {
		SceneGraph::getInstance().AddLight(SceneGraph::getInstance().GetActor(actorName));
	}

	return false;
}

bool Actor::DeinitalizeLight()
{
	if (SceneGraph::getInstance().GetLightExist(SceneGraph::getInstance().GetActor(actorName))) {
		SceneGraph::getInstance().RemoveLight(SceneGraph::getInstance().GetActor(actorName));
	}

	return false;
}

void Actor::RemoveAllComponents() {
	DeinitalizeLight();
	components.clear();
}


void Actor::pushToSceneGraphWorker(Ref<Component> component)
{


	Ref<MeshComponent> mesh = std::dynamic_pointer_cast<MeshComponent>(component);
	Ref<Animation> animation = std::dynamic_pointer_cast<Animation>(component);

	if (mesh) {
		SceneGraph::getInstance().pushMeshToWorker(mesh);
	}
	else if (animation) {

		SceneGraph::getInstance().pushAnimationToWorker(animation);

	}


}


//
void Actor::ListComponents() const {
	std::cout << typeid(*this).name() << " contains the following components:\n";
	for (auto &component : components) {
		std::cout << typeid(*component).name() << std::endl;
	}
	std::cout << '\n';
}


Matrix4 Actor::GetModelMatrix(Ref<Component> camera_) {
	Ref<TransformComponent> transform = GetComponent<TransformComponent>();
	modelMatrix = transform ? transform->GetTransformMatrix() : Matrix4();

	// multiples the parents model matrix by the childs (hierarchly/recursivly multiples parent matrix/transform)
	// TODO: find something better for parent/child movement
	if (parent && dynamic_cast<Actor*>(parent)) {
		modelMatrix = dynamic_cast<Actor*>(parent)->GetModelMatrix(camera_) * modelMatrix;
	}

	return modelMatrix;
}

Vec3 Actor::GetPositionFromHierarchy(Ref<Component> camera_)
{
	Ref<CameraComponent> cameraComponent_ = std::dynamic_pointer_cast<CameraComponent>(camera_);

	Ref<TransformComponent> transform = GetComponent<TransformComponent>();


	Vec3 rPos = transform ? transform->GetPosition() : Vec3();



	if (parent) {
		rPos = dynamic_cast<Actor*>(parent)->GetPositionFromHierarchy(camera_) + rPos;
	}
	else if (camera_) {
		rPos = -cameraComponent_->GetUserActor()->GetComponent<TransformComponent>()->GetPosition() + rPos;
	}
	return rPos;
}



bool Actor::GetIntersectTriangles(Vec3 start, Vec3 dir, Vec3* intersectSpot) {

	//if (GetComponent<MeshComponent>() == nullptr) return false;
	////std::cout << actorName << std::endl;
	//modelMatrix = GetModelMatrix();

	//std::vector<Vec3> vertices = GetComponent<MeshComponent>()->getMeshVertices();
	//for (size_t i = 0; i + 2 < vertices.size(); i += 3) {

	//	//Vertices translated to have same transform as rendered
	//	Vec3 v0 = Vec3(modelMatrix * Vec4(vertices[i], 1.0f));
	//	Vec3 v1 = Vec3(modelMatrix * Vec4(vertices[i + 1], 1.0f));
	//	Vec3 v2 = Vec3(modelMatrix * Vec4(vertices[i + 2], 1.0f));

	//	Raycast::Triangle tri = { v0, v1, v2 };

	//	//Eliminate triangles in lower cost function (cone)
	//	if (!Raycast::isInRayCone(tri.getCenter(), start, dir, 0.98985f)) continue;


	//	//Eliminate triangles not facing the camera (something called Backfacing)
	//	// was having issues before where meshes with larger vertices would cause VMath's normailze to divide by zero, this should hopefully fix it
	//	Vec3 edge1 = v1 - v0;
	//	Vec3 edge2 = v2 - v0;
	//	Vec3 crossProduct = VMath::cross(edge1, edge2);

	//	float crossMag = VMath::mag(crossProduct);
	//	if (crossMag < VERY_SMALL) {
	//		continue;
	//	}

	//	Vec3 normal = crossProduct / crossMag;
	//	if (VMath::dot(normal, dir) > 0) {
	//		continue; 
	//	}
	//	
	//	//Check if intersecting (Expensive)
	//	if (Raycast::intersectRayTri(tri, start, dir)) {

	//		//if a pointer to intersectSpot is included, send over the intersected triangle
	//		if (intersectSpot) {
	//			intersectSpot->x = tri.getCenter().x;
	//			intersectSpot->y = tri.getCenter().y;
	//			intersectSpot->z = tri.getCenter().z;
	//		}

	//		return true; 
	//	}

	//	
	//}

	return false;
}


	

#include "CameraComponent.h"

CameraComponent::CameraComponent(Ref<Actor> userActor, float fovy, float aspectRatio, float nearClipPlane, float farClipPlane) : Component(nullptr)
{

	parentActor = userActor;
	projectionMatrix = MMath::perspective(fovy, aspectRatio, nearClipPlane, farClipPlane);

	viewMatrix.loadIdentity();
	//viewMatrix = MMath::lookAt(Vec3(0.0f, 0.0f, 5.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));

}

CameraComponent::~CameraComponent()
{

}



void CameraComponent::OnDestroy() {
	std::cout << "deleting cam's actor pointer" << std::endl;
	parentActor = nullptr;
}

void CameraComponent::Update(const float deltaTime) {
	std::cout << "Hello from Update " << deltaTime << '\n';
}

void CameraComponent::Render()const {}

bool CameraComponent::OnCreate()
{
	Ref<TransformComponent> tc = parentActor->GetComponent<TransformComponent>();

	//
	if (tc != nullptr) {
		viewMatrix = tc->GetTransformMatrix();
		//viewMatrix.print("View Matrix");
	}

	return true;
}

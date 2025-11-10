#include "CameraComponent.h"

CameraComponent::CameraComponent(Ref<Actor> userActor_, float fovy, float aspectRatio, float nearClipPlane, float farClipPlane) : 
	Component(nullptr), m_fov(fovy), m_aspectRatio(aspectRatio), m_nearClipPlane(nearClipPlane), m_farClipPlane(farClipPlane)
{

	userActor = userActor_;
	projectionMatrix = MMath::perspective(m_fov, m_aspectRatio, m_nearClipPlane, m_farClipPlane);

	viewMatrix.loadIdentity();
	//viewMatrix = MMath::lookAt(Vec3(0.0f, 0.0f, 5.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));

}

CameraComponent::~CameraComponent()
{

}



void CameraComponent::OnDestroy() {
	std::cout << "deleting cam's actor pointer" << std::endl;
	userActor = nullptr;
}

void CameraComponent::Update(const float deltaTime) {
	std::cout << "Hello from Update " << deltaTime << '\n';
}

void CameraComponent::Render()const {}

bool CameraComponent::OnCreate()
{
	fixCameraToTransform();
	return true;
}

void CameraComponent::fixCameraToTransform() {
	if (userActor->GetComponent<TransformComponent>()) {
		Ref<TransformComponent> transform = userActor->GetComponent<TransformComponent>();

		position = transform->GetPosition();
		orientation = transform->GetQuaternion();

		Matrix4 cameraWorldTransform = MMath::translate(position) * MMath::toMatrix4(orientation);

		viewMatrix = MMath::inverse(cameraWorldTransform);


	}
}
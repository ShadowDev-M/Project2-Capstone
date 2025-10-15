#include "CameraComponent.h"

CameraComponent::CameraComponent(Ref<Actor> userActor, float fovy, float aspectRatio, float nearClipPlane, float farClipPlane) : 
	Component(nullptr), m_fov(fovy), m_aspectRatio(aspectRatio), m_nearClipPlane(nearClipPlane), m_farClipPlane(farClipPlane)
{

	parentActor = userActor;
	projectionMatrix = MMath::perspective(m_fov, m_aspectRatio, m_nearClipPlane, m_farClipPlane);

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

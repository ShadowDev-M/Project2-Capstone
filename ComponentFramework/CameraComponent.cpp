#include "pch.h"
#include "CameraComponent.h"
#include "ScreenManager.h"

CameraComponent::CameraComponent(Ref<Actor> userActor_, float fovy, float aspectRatio, float nearClipPlane, float farClipPlane) : 
	Component(nullptr), m_fov(fovy), m_aspectRatio(aspectRatio), m_nearClipPlane(nearClipPlane), m_farClipPlane(farClipPlane)
{

	userActor = userActor_;
	projectionMatrix = MMath::perspective(m_fov, m_aspectRatio, m_nearClipPlane, m_farClipPlane);
	viewMatrix.loadIdentity();
	
	// register the camera to the resize dispatcher
	resizeCallbackID = ScreenManager::getInstance().OnRenderResize(
		[this](int w, int h) {
			m_aspectRatio = static_cast<float>(w) / static_cast<float>(h);
			UpdateProjectionMatrix();
		}
	);
}

CameraComponent::~CameraComponent() {
	OnDestroy();
}

void CameraComponent::OnDestroy() {
	// unregister the camera from the resize dispatcher
	if (resizeCallbackID != -1) {
		ScreenManager::getInstance().RemoveRenderResizeCallback(resizeCallbackID);
		resizeCallbackID = -1;
	}

	userActor = nullptr;
}

void CameraComponent::Update(const float deltaTime) {}

void CameraComponent::Render()const {}

bool CameraComponent::OnCreate()
{
	fixCameraToTransform();
	return true;
}

void CameraComponent::fixCameraToTransform() {
	if (userActor->GetComponent<TransformComponent>()) {
		Ref<TransformComponent> transform = userActor->GetComponent<TransformComponent>();

		Vec3 position = transform->GetPosition();
		Quaternion orientation = transform->GetOrientation();

		Matrix4 cameraWorldTransform = MMath::translate(position) * MMath::toMatrix4(orientation);

		viewMatrix = MMath::inverse(cameraWorldTransform);


	}
}
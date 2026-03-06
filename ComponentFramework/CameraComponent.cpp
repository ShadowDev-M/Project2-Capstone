#include "pch.h"
#include "CameraComponent.h"
#include "InputManager.h"


CameraComponent::CameraComponent(Ref<Actor> userActor_, float fovy, float aspectRatio, float nearClipPlane, float farClipPlane, bool orthographicState) :
	Component(nullptr), m_fov(fovy), m_aspectRatio(aspectRatio), m_nearClipPlane(nearClipPlane), m_farClipPlane(farClipPlane), isOrthographic(orthographicState)
{
	userActor = userActor_;
	UpdateProjectionMatrix();
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

void CameraComponent::UpdateProjectionMatrix()
{

	if (isOrthographic) {
		Vec2 windowSize = Vec2(InputManager::getInstance().getMouseMap()->dockingSize.x, InputManager::getInstance().getMouseMap()->dockingSize.y);


		//pos of top left corner of docking window
		GLint xPos = (GLint)InputManager::getInstance().getMouseMap()->dockingPos.x;
		GLint yPos = (GLint)InputManager::getInstance().getMouseMap()->dockingPos.y;

		GLsizei xSize;
		GLsizei ySize;




		// Calculate scaled dimensions based on aspect ratio
		if (windowSize.x / m_aspectRatio <= windowSize.y)
		{
			xSize = (GLsizei)windowSize.x;
			ySize = (GLsizei)(windowSize.x / m_aspectRatio);
		}
		else
		{
			ySize = (GLsizei)windowSize.y;
			xSize = (GLsizei)(windowSize.y * m_aspectRatio);
		}


		//Add to the position to move to where the image is centred (non used space in the window would be counted otherwise)
		ImVec2 imagePos = ImVec2((windowSize.x - xSize) * 0.5f, (windowSize.y - ySize) * 0.5f);
		xPos += (GLint)imagePos.x;
		yPos += (GLint)imagePos.y;

		if (!userActor || !userActor->GetComponent<TransformComponent>()) return;
		float z = userActor->GetComponent<TransformComponent>()->GetPosition().z;
		float fovRad = m_fov * M_PI / 180.0f;   // if m_fov is in degrees
		float halfSize = z * tanf(0.5 * fovRad);
		halfSize *= 1.75f;

		projectionMatrix = MMath::orthographic(
			-halfSize, halfSize,      // X
			-halfSize / m_aspectRatio, halfSize/ m_aspectRatio,      // Y
			-100.0f, m_farClipPlane           // Z
		);
		//projectionMatrix = MMath::orthographic(??, ??, ??, ??, m_nearClipPlane, m_farClipPlane);
	}
	else {
		projectionMatrix = MMath::perspective(m_fov, m_aspectRatio, m_nearClipPlane, m_farClipPlane);
	}
}

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
		
		//for z
		if (isOrthographic) UpdateProjectionMatrix();

	}
}
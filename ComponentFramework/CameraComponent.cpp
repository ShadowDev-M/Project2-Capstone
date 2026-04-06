#include "pch.h"
#include "CameraComponent.h"
#include "InputManager.h"
#include "ScreenManager.h"

CameraComponent::CameraComponent(Component* parent_, ProjectionType type_, float fov_, float nearClipPlane_, float farClipPlane_, float orthoSize_) :
	Component(parent_), m_projectionType(type_), m_fov(fov_), m_nearClip(nearClipPlane_), m_farClip(farClipPlane_), m_orthoSize(orthoSize_) {}

bool CameraComponent::OnCreate()
{
	if (isCreated) return true;
	isCreated = true;
	return true;
}

void CameraComponent::OnDestroy() {
	isCreated = false;
}

bool CameraComponent::isMainCamera() const {
	// getting parent/actor of the component
	if (!parent) return false;
	Actor* owner = dynamic_cast<Actor*>(parent);
	return owner && owner->getTag() == "MainCamera";
}

Matrix4 CameraComponent::GetProjectionMatrix() const {
	float aspectRatio = ScreenManager::getInstance().getRenderAspectRatio();

	if (m_projectionType == ProjectionType::Orthographic) {
		float h = m_orthoSize;
		float w = h * aspectRatio;
		//instead of near clip, farclip like perspective, orthographic needs -farclip, farclip because instead of starting from the camera, it starts at a certain 'slice' of space
		// then it needs to render things both in front of the slice (-farclip), and behind the slice (farclip), otherwise you will have weird rendering cuts when rotating or moving
		return MMath::orthographic(-w, w, -h, h, -m_farClip, m_farClip);
	}
	else {
		return MMath::perspective(m_fov, aspectRatio, m_nearClip, m_farClip);
	}
}

Matrix4 CameraComponent::GetViewMatrix() const {
	// getting the transform from the parent
	if (!parent) return Matrix4();
	Actor* owner = dynamic_cast<Actor*>(parent);
	if (!owner) return Matrix4();
	Ref<TransformComponent> TC = owner->GetComponent<TransformComponent>();
	if (!TC) return Matrix4();

	// getting the position, forward, and up from the actors model matrix 
	// colum 0 is the right 0-2
	// colum 1 is up 4-6
	// colum 2 is -forward 8-10 (because z is negative in opengl)
	// colum 3 is position 12-14

	Matrix4 world = owner->GetModelMatrix();
	Vec3 up = world.getColumn(Matrix4::Colunm::one);
	Vec3 forward = -world.getColumn(Matrix4::Colunm::two);
	Vec3 pos = world.getColumn(Matrix4::Colunm::three);
	return MMath::lookAt(pos, pos + forward, up);
}

Vec3 CameraComponent::getWorldPosition() const
{
	// getting the transform from the parent
	if (!parent) return Vec3();
	Actor* owner = dynamic_cast<Actor*>(parent);
	if (!owner) return Vec3();
	Ref<TransformComponent> TC = owner->GetComponent<TransformComponent>();
	if (!TC) return Vec3();

	// getting the position, forward, and up from the actors model matrix 
	// colum 0 is the right 0-2
	// colum 1 is up 4-6
	// colum 2 is -forward 8-10 (because z is negative in opengl)
	// colum 3 is position 12-14

	Matrix4 world = owner->GetModelMatrix();
	Vec3 pos = world.getColumn(Matrix4::Colunm::three);
	return pos;
}

Vec3 CameraComponent::getWorldForward() const
{
	// getting the transform from the parent
	if (!parent) return Vec3();
	Actor* owner = dynamic_cast<Actor*>(parent);
	if (!owner) return Vec3();
	Ref<TransformComponent> TC = owner->GetComponent<TransformComponent>();
	if (!TC) return Vec3();

	// getting the position, forward, and up from the actors model matrix 
	// colum 0 is the right 0-2
	// colum 1 is up 4-6
	// colum 2 is -forward 8-10 (because z is negative in opengl)
	// colum 3 is position 12-14

	Matrix4 world = owner->GetModelMatrix();
	Vec3 forward = -world.getColumn(Matrix4::Colunm::two);
	return forward;
}
#pragma once
#include "TransformComponent.h"
#include "Actor.h"

enum class ProjectionType {
	Perspective,
	Orthographic
};                                          

class CameraComponent : public Component {
	// deleting copy and move constructers, setting up singleton
	CameraComponent(const CameraComponent&) = delete;
	CameraComponent(CameraComponent&&) = delete;
	CameraComponent& operator=(const CameraComponent&) = delete;
	CameraComponent& operator=(CameraComponent&&) = delete;

public:
	CameraComponent(Component* parent_, ProjectionType type_ = ProjectionType::Perspective, float fov_ = 60.0f, float nearClipPlane_ = 0.03f, float farClipPlane= 1000.0f, float orthoSize_ = 5.0f);
	~CameraComponent() = default;
	
	bool OnCreate();
	void OnDestroy();
	void Update(const float deltaTime_) {};
	void Render() const {};

	bool isMainCamera() const;

	Matrix4 GetProjectionMatrix() const;
	Matrix4 GetViewMatrix() const;

	// helper functions if the camera is ever parented and need to get world transform
	Vec3 getWorldPosition() const;
	Vec3 getWorldForward() const;

	// getters and setters for the cameras member variables
	ProjectionType getType() const { return m_projectionType; }
	float getFOV() const { return m_fov; }
	float getNearClipPlane() const { return m_nearClip; }
	float getFarClipPlane() const { return m_farClip; }
	float getOrthoSize() const { return m_orthoSize; }
	void setType(ProjectionType type_) { m_projectionType = type_; }
	void setFOV(float fov) { m_fov = fov; }
	void setNearClipPlane(float near) { m_nearClip = std::max(0.0001f, near); }
	void setFarClipPlane(float far) { m_farClip = far; }
	void setOrthoSize(float size) { m_orthoSize = size; }

private:

	// camera member variables
	ProjectionType m_projectionType = ProjectionType::Perspective;
	float m_fov = 60.0f;
	float m_nearClip = 0.03f;
	float m_farClip = 10000.0f;
	float m_orthoSize = 5.0f;
};


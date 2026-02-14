#pragma once
#include "TransformComponent.h"
#include "Actor.h"

using namespace MATH;

class CameraComponent : public Component {
private:
	

	Ref<Actor> userActor;

	Matrix4 projectionMatrix;
	Matrix4 viewMatrix;

	// camera member variables to be accesed by imgui
	float m_fov, m_aspectRatio, m_nearClipPlane, m_farClipPlane;

public:
	CameraComponent(Ref<Actor> userActor_ = nullptr, float fovy = 45.0f, float aspectRatio = (16.0f / 9.0f), float nearClipPlane = 0.5f, float farClipPlane= 100.0f);
	~CameraComponent();
	bool OnCreate();

	Ref<Actor> GetUserActor() { return userActor; }

	Ref<TransformComponent> GetUserActorTransform() { return userActor->GetComponent<TransformComponent>(); }


	Matrix4 GetProjectionMatrix() const { return projectionMatrix; }
	//void SetProjectionMatrix(const Matrix4& projectionMatrix_) { projectionMatrix = projectionMatrix_; }

	Matrix4 GetViewMatrix() const { return viewMatrix; }
	//
	//void SetViewMatrix(const Quaternion& orientation_, const Vec3& position_) { orientation = orientation_, position = position_; }

	void fixCameraToTransform();

	void OnDestroy();
	void Update(const float deltaTime_);
	void Render() const;


	// setters and getters for the cameras member variables
	
	void UpdateProjectionMatrix() {
		projectionMatrix = MMath::perspective(m_fov, m_aspectRatio, m_nearClipPlane, m_farClipPlane);
	}
	
	Ref<Actor> getUserActor() const { return userActor; }
	float getFOV() const { return m_fov; }
	float getAspectRatio() const { return m_aspectRatio; }
	float getNearClipPlane() const { return m_nearClipPlane; }
	float getFarClipPlane() const { return m_farClipPlane; }

	void setUserActor(Ref<Actor> actor_) { userActor = actor_; }

	void setFOV(float fov_) { 
		m_fov = fov_;
		UpdateProjectionMatrix();
	}
	void setAspectRatio(float aspectRatio_) { 
		m_aspectRatio = aspectRatio_; 
		UpdateProjectionMatrix();
	}
	void setNearClipPlane(float nearClipPlane_) { 
		m_nearClipPlane = nearClipPlane_; 
		UpdateProjectionMatrix();
	}
	void setFarClipPlane(float farClipPlane_) { 
		m_farClipPlane = farClipPlane_; 
		UpdateProjectionMatrix();
	}

};


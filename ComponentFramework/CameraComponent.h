#pragma once
#include "Component.h"
#include <Vector.h>
#include <MMath.h>
#include <Quaternion.h>
#include "TransformComponent.h"
#include "Actor.h"

using namespace MATH;

class CameraComponent : public Component {
private:
	

	Ref<Actor> parentActor;

	Matrix4 projectionMatrix;
	Matrix4 viewMatrix;

	// camera member variables to be accesed by imgui
	float m_fov, m_aspectRatio, m_nearClipPlane, m_farClipPlane;

	//
	Quaternion orientation;
	Vec3 position;

public:
	CameraComponent(Ref<Actor> userActor, float fovy, float aspectRatio, float nearClipPlane, float farClipPlane);
	~CameraComponent();
	bool OnCreate();

	Ref<Actor> GetUserActor() { return parentActor; }

	Matrix4 GetProjectionMatrix() const { return projectionMatrix; }
	//void SetProjectionMatrix(const Matrix4& projectionMatrix_) { projectionMatrix = projectionMatrix_; }

	Matrix4 GetViewMatrix() const { return viewMatrix; }
	//
	//void SetViewMatrix(const Quaternion& orientation_, const Vec3& position_) { orientation = orientation_, position = position_; }

	void fixCameraToTransform() {
		
		if (parentActor->GetComponent<TransformComponent>()) {
			position = Vec3();

			orientation = parentActor->GetComponent<TransformComponent>()->GetQuaternion();
			viewMatrix = MMath::translate((position)) * MMath::toMatrix4((orientation));


		}
	}

	void OnDestroy();
	void Update(const float deltaTime_);
	void Render() const;


	// setters and getters for the cameras member variables
	
	void UpdateProjectionMatrix() {
		projectionMatrix = MMath::perspective(m_fov, m_aspectRatio, m_nearClipPlane, m_farClipPlane);
	}

	float getFOV() const { return m_fov; }
	float getAspectRatio() const { return m_aspectRatio; }
	float getNearClipPlane() const { return m_nearClipPlane; }
	float getFarClipPlane() const { return m_farClipPlane; }

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


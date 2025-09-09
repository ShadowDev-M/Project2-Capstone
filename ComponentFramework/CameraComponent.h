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
};


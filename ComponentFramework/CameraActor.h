#pragma once
#include "Actor.h"
#include <Vector.h>
#include <MMath.h>
#include <Quaternion.h>
#include "TransformComponent.h"

using namespace MATH;

class CameraActor : public Actor {
private:
	
	Matrix4 projectionMatrix;
	Matrix4 viewMatrix;

	//
	Quaternion orientation;
	Vec3 position;

public:
	CameraActor(Actor* parent_, float fovy, float aspectRatio, float nearClipPlane, float farClipPlane);
	~CameraActor();
	bool OnCreate();

	Matrix4 GetProjectionMatrix() const { return projectionMatrix; }
	//void SetProjectionMatrix(const Matrix4& projectionMatrix_) { projectionMatrix = projectionMatrix_; }

	Matrix4 GetViewMatrix() const { return viewMatrix; }
	//
	//void SetViewMatrix(const Quaternion& orientation_, const Vec3& position_) { orientation = orientation_, position = position_; }

	void fixCameraToTransform() {
		if (GetComponent<TransformComponent>()) {
			position = Vec3();
			
			orientation = GetComponent<TransformComponent>()->GetQuaternion();
			viewMatrix = MMath::translate((position))*MMath::toMatrix4((orientation));
			
			
		}
	}
};


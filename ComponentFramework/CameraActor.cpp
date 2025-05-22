#include "CameraActor.h"

CameraActor::CameraActor(Actor* parent_, float fovy, float aspectRatio, float nearClipPlane, float farClipPlane) : Actor(parent_)
{
	projectionMatrix = MMath::perspective(fovy, aspectRatio, nearClipPlane, farClipPlane);	
	
	viewMatrix.loadIdentity();
	actorName = "camera";
	//viewMatrix = MMath::lookAt(Vec3(0.0f, 0.0f, 5.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));

}

CameraActor::~CameraActor()
{

}

bool CameraActor::OnCreate()
{
	Ref<TransformComponent> tc = GetComponent<TransformComponent>();

	//
	if (tc != nullptr) {
		viewMatrix = tc->GetTransformMatrix();
		viewMatrix.print("View Matrix");
	}

	return true;
}

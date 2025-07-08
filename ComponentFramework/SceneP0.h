#ifndef SCENEP0_H
#define SCENEP0_H
#include "Scene.h"
#include "Vector.h"
#include <Matrix.h>
#include "Actor.h"
#include "CameraActor.h"
#include <vector>

#include "SceneGraph.h"

#include "TransformComponent.h"
#include "CollisionComponent.h"
#include "CollisionSystem.h"

#include "AssetManager.h"

using namespace MATH;

/// Forward declarations 
union SDL_Event;

class SceneP0 : public Scene {
private:
	
	SceneGraph sceneGraph;
	CollisionSystem collisionSystem;

	Vec3 lightPos;

	Ref<CameraActor> camera;

	bool drawInWireMode;
	Ref<Actor> selectedAsset;
	float debugMoveSpeed = 0.5f;

public:
	explicit SceneP0();
	virtual ~SceneP0();

	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float deltaTime) override;
	virtual void Render() const override;
	virtual void HandleEvents(const SDL_Event &sdlEvent) override;
	
};


#endif // SCENE2_H
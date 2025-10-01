#ifndef SCENE3GUI_H
#define SCENE3GUI_H
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

class Scene3GUI : public Scene {
private:
	CollisionSystem collisionSystem;
	Vec3 lightPos;
	Ref<CameraActor> camera;
	bool drawInWireMode;

public:
	explicit Scene3GUI();
	virtual ~Scene3GUI();

	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float deltaTime) override;
	virtual void Render() const override;
	virtual void HandleEvents(const SDL_Event &sdlEvent) override;

};


#endif // Scene3GUI
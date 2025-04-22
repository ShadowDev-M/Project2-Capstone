#ifndef SCENE2_H
#define SCENE2_H
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

class Scene2g : public Scene {
private:
	
	SceneGraph sceneGraph;
	CollisionSystem collisionSystem;

	Vec3 lightPos;

	CameraActor* camera;

	bool drawInWireMode;


public:
	explicit Scene2g();
	virtual ~Scene2g();

	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float deltaTime) override;
	virtual void Render() const override;
	virtual void HandleEvents(const SDL_Event &sdlEvent) override;

};


#endif // SCENE2_H
#ifndef SCENE1_H
#define SCENE1_H
#include "Scene.h"
#include "Vector.h"
#include <Matrix.h>
#include "Actor.h"
#include "CameraActor.h"
#include <vector>

#include <unordered_map>
#include <string>

#include "MeshComponent.h"
#include "ShaderComponent.h"
#include "TransformComponent.h"
#include "MaterialComponent.h"

#include "PhysicsComponent.h"
#include "CollisionComponent.h"
#include "CollisionSystem.h"

using namespace MATH;

/// Forward declarations 
union SDL_Event;

class Scene1g : public Scene {
private:
	
	CollisionSystem collisionSystem;


	

	Ref<Actor> board;

	// once assetmanager is setup, move unorderedmap there and add additional functions for adding, removing, and updating all actors
	// unordered map that will hold all actors in the scene + their names
	std::unordered_map<std::string, Ref<Actor>> Actors;

	Vec3 lightPos;

	CameraActor* camera;

	bool drawInWireMode;

	// no asset manager just yet so setting up shared components in the scene
	
	// Shader
	Ref<ShaderComponent> phongShader;

	// Meshes
	Ref<MeshComponent> pawnMesh;
	Ref<MeshComponent> bishopMesh;
	Ref<MeshComponent> kingMesh;
	Ref<MeshComponent> knightMesh;
	Ref<MeshComponent> queenMesh;
	Ref<MeshComponent> rookMesh;
	Ref<MeshComponent> planeMesh;

	// Materials
	Ref<MaterialComponent> blackChess;
	Ref<MaterialComponent> whiteChess;
	Ref<MaterialComponent> chessBoard;


public:
	explicit Scene1g();
	virtual ~Scene1g();

	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float deltaTime) override;
	virtual void Render() const override;
	
	// Render all actors in the unordered map
	void RenderMap(const std::string& actorName) const;
	
	virtual void HandleEvents(const SDL_Event &sdlEvent) override;

};


#endif // SCENE1_H
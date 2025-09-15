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

#include "HierarchyWindow.h"
#include "InspectorWindow.h"
#include "AssetManagerWindow.h"

using namespace MATH;

/// Forward declarations 
union SDL_Event;

class Scene3GUI : public Scene {
private:
	
	SceneGraph sceneGraph;
	CollisionSystem collisionSystem;

	Vec3 lightPos;


	Ref<CameraActor> camera;

	bool drawInWireMode;
	Ref<Actor> selectedAsset;
	float debugMoveSpeed = 0.5f;

	// Window States
	mutable bool showDemoWindow = false;
	mutable bool showHierarchyWindow = true;
	mutable bool showInspectorWindow = true;
	mutable bool showAssetmanagerWindow = true;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	
	// save file dialog states (switch to own class)
	mutable bool showSaveFileDialog = false;
	mutable bool showLoadFileDialog = false;
	std::string saveFileName = "";
	void ShowSaveDialog();
	void ShowLoadDialog();

	// using unqiue pointers for automatic memory management
	// could switch to shared pointers if we ever intend on having this window in multiple scenes
	std::unique_ptr<HierarchyWindow> hierarchyWindow;
	std::unique_ptr<InspectorWindow> inspectorWindow;
	std::unique_ptr<AssetManagerWindow> assetManagerWindow;
	
	


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
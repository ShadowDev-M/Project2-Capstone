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
	
	SceneGraph sceneGraph;
	CollisionSystem collisionSystem;

	Vec3 lightPos;

	Ref<CameraActor> camera;

	bool drawInWireMode;
	Ref<Actor> selectedAsset;
	float debugMoveSpeed = 0.5f;

	// Window States
	mutable bool show_demo_window = false;
	mutable bool show_hierarchy_window = false;
	mutable bool show_inspector_window = false;
	mutable bool show_assetmanager_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Possibly move each unique window to its own class

	//// Hierarchy window
	void ShowHierarchyWindow(bool* p_open);
	// recursive function for actually rendering/drawing the nodes
	void DrawActorNode(const std::string& actorName, Ref<Actor> actor);
	// recursive functions that help with selecting and searching for a child node
	bool HasFilteredChild(Component* parent);
	bool HasSelectedChild(Component* parent);

	// refactored function to get a map of all child actors
	std::unordered_map<std::string, Ref<Actor>> GetChildActors(Component* parent);
	
	mutable bool show_only_selected = false;

	// text filter for the hierarchy window
	ImGuiTextFilter filter;
	////


	//// Inspector Window
	void ShowInspectorWindow(bool* p_open);
	
	// component functions
	void DrawTransformComponent(Ref<TransformComponent> transform);
	void DrawMeshComponent(Ref<MeshComponent> mesh);
	void DrawMaterialComponent(Ref<MaterialComponent> material);
	void DrawShaderComponent(Ref<ShaderComponent> shader);

	
	//// Asset Manager
	void ShowAssetManagerWindow(bool* p_open);
	void DrawAssetThumbnail(const std::string& assetName, Ref<Component> asset);
	ImGuiTextFilter assetFilter;
	const int thumbnail_size = 64;
	const int padding = 10;


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
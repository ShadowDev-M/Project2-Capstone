#pragma once
#include "imgui.h"
#include "SceneGraph.h"

class InspectorWindow
{
	// delete the move and copy constructers
	InspectorWindow(const InspectorWindow&) = delete;
	InspectorWindow(InspectorWindow&&) = delete;
	InspectorWindow& operator = (const InspectorWindow&) = delete;
	InspectorWindow& operator = (InspectorWindow&&) = delete;
private:
	// pointer to scenegraph
	SceneGraph* sceneGraph;

	// thumbnail size, assetmanager also has this, possibly find a way to make it so they share the same thing incase I edit
	const int thumbnail_size = 64;

	// component functions
	void DrawTransformComponent(Ref<TransformComponent> transform);
	void DrawMeshComponent(Ref<MeshComponent> mesh);
	void DrawMaterialComponent(Ref<MaterialComponent> material);
	void DrawShaderComponent(Ref<ShaderComponent> shader);


public:
	explicit InspectorWindow(SceneGraph* sceneGraph_);
	~InspectorWindow() {}

	void ShowInspectorWindow(bool* p_open);
};


#pragma once

#include "SceneGraph.h"
#include "EditorManager.h"

class SceneWindow
{
	// delete the move and copy constructers
	SceneWindow(const SceneWindow&) = delete;
	SceneWindow(SceneWindow&&) = delete;
	SceneWindow& operator = (const SceneWindow&) = delete;
	SceneWindow& operator = (SceneWindow&&) = delete;
private:
	// imguizmo variables
	ImGuizmo::OPERATION currentGizmoOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE currentGizmoMode = ImGuizmo::WORLD;
	bool useGizmoSnap = false;
	float gizmoSnapValues[3] = { 1.0f, 1.0f, 1.0f };

	bool showDebugGizmos = true;

	void DrawGizmos(ImVec2 scaledTexture_, ImVec2 imagePos_);

	void ConvertMat4toFloatArray(const Matrix4& matrix_, float* array_);
	void ConvertFloatArraytoMat4(Matrix4& matrix_, float* array_);

	// pointer to scenegraph
	SceneGraph* sceneGraph;

	struct PreviewState {
		Ref<Actor>      actor;
		Ref<Component>  originalComponent;
		std::string     componentType;
		bool            active = false;
		ImVec2          appliedAt = { 0.0f, 0.0f };
	};
	PreviewState previewState;
	void ApplyPreview(Ref<Actor> actor, const EditorManager::ProjectDragPayload* data);
	void RevertPreview();
	void CommitDrop(Ref<Actor> actor, const EditorManager::ProjectDragPayload* data);
	void dropAssetOnScene();

public:
	explicit SceneWindow(SceneGraph* sceneGraph_);
	~SceneWindow() {}

	void ShowSceneWindow(bool* pOpen);
};
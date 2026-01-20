#pragma once

#include "SceneGraph.h"

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

	void DrawGizmos(ImVec2 scaledTexture_, ImVec2 imagePos_);

	void ConvertMat4toFloatArray(const Matrix4& matrix_, float* array_);
	void ConvertFloatArraytoMat4(Matrix4& matrix_, float* array_);

	// pointer to scenegraph
	SceneGraph* sceneGraph;

	template<typename ComponentTemplate>
	void dropAssetOnScene();

public:
	explicit SceneWindow(SceneGraph* sceneGraph_);
	~SceneWindow() {}

	void ShowSceneWindow(bool* pOpen);
};



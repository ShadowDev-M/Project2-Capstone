#pragma once

#include "imgui.h"
#include "imgui_stdlib.h"
#include "SceneGraph.h"

class DockingWindow
{
	// delete the move and copy constructers
	DockingWindow(const DockingWindow&) = delete;
	DockingWindow(DockingWindow&&) = delete;
	DockingWindow& operator = (const DockingWindow&) = delete;
	DockingWindow& operator = (DockingWindow&&) = delete;
private:
	// text filter for the hierarchy window
	ImGuiTextFilter filter;

	// pointer to scenegraph
	SceneGraph* sceneGraph;

	template<typename ComponentTemplate>
	void dropAssetOnScene();

public:
	explicit DockingWindow(SceneGraph* sceneGraph_);
	~DockingWindow() {}

	void ShowDockingWindow(bool* pOpen);

	/// <summary>
	/// gets called when in a scenes on destroy to make sure the filter is cleared 
	/// (just adding this incase we decide to use the same window for multiple scenes)
	/// </summary>
	void ClearFilter() { filter.Clear(); }
};



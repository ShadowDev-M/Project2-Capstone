#pragma once
#include "imgui.h"
#include "SceneGraph.h"

class HierarchyWindow
{
	// delete the move and copy constructers
	HierarchyWindow(const HierarchyWindow&) = delete;
	HierarchyWindow(HierarchyWindow&&) = delete;
	HierarchyWindow& operator = (const HierarchyWindow&) = delete;
	HierarchyWindow& operator = (HierarchyWindow&&) = delete;
private:
	// state to show selected actors
	mutable bool showOnlySelected = false;

	// text filter for the hierarchy window
	ImGuiTextFilter filter;

	// pointer to scenegraph
	SceneGraph* sceneGraph;

	// recursive function for actually rendering/drawing the nodes
	void DrawActorNode(const std::string& actorName, Ref<Actor> actor);
	
	// recursive functions that help with selecting and searching for a child node
	bool HasFilteredChild(Component* parent);
	bool HasSelectedChild(Component* parent);

	// refactored function to get a map of all child actors
	std::unordered_map<std::string, Ref<Actor>> GetChildActors(Component* parent);

public:
	explicit HierarchyWindow(SceneGraph* sceneGraph_);
	~HierarchyWindow() {}

	void ShowHierarchyWindow(bool* pOpen);

	/// <summary>
	/// gets called when in a scenes on destroy to make sure the filter is cleared 
	/// (just adding this incase we decide to use the same window for multiple scenes)
	/// </summary>
	void ClearFilter() { filter.Clear(); }
};


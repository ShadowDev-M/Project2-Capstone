#pragma once
#include "imgui.h"
#include "imgui_stdlib.h"
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
	void DrawActorNode(const std::string& actorName_, Ref<Actor> actor_);
	
	// recursive functions that help with selecting and searching for a child node
	bool HasFilteredChild(Component* parent);
	bool HasSelectedChild(Component* parent);

	// refactored function to get a map of all child actors
	std::unordered_map<std::string, Ref<Actor>> GetChildActors(Component* parent);

	// rename, duplicate, and re-parenting functions
	void DuplicateActor(Ref<Actor> original_);
	Ref<Actor> DeepCopyActor(const std::string& newName_, Ref<Actor> original_);
	std::string GenerateDuplicateName(const std::string& originalName_);
	void HandleDragDrop(const std::string& actorName_, Ref<Actor> actor_);

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


#include "HierarchyWindow.h"

HierarchyWindow::HierarchyWindow(SceneGraph* sceneGraph_) : sceneGraph(sceneGraph_) {}

void HierarchyWindow::ShowHierarchyWindow(bool* p_open)
{

	// Some of the stuff here will be changed after to just read off an XML file (like getting all the actors names and if they are parented/children)
	// just doing it without XML stuff for now since I don't fully understand it yet

	if (ImGui::Begin("Hierarchy", p_open, ImGuiWindowFlags_MenuBar)) {

		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("Options")) {

				ImGui::Checkbox("Show Only Selected", &show_only_selected);

				if (ImGui::MenuItem("Select All")) {
					std::vector<std::string> allActorNames = sceneGraph->GetAllActorNames();
					for (const auto& actorName : allActorNames) {
						Ref<Actor> actor = sceneGraph->GetActor(actorName);

						sceneGraph->debugSelectedAssets.emplace(actorName, actor);
					}
				}

				if (ImGui::MenuItem("Clear All Selected")) {
					sceneGraph->debugSelectedAssets.clear();
				}


				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		/*if (IsWindowFocused()) {
			SetKeyboardFocusHere();
			filter.Clear();
		} */

		filter.Draw("##HierarchyFilter", -1.0f);
		ImGui::Separator();

		//// Most of this will be changed after to just read off an XML for the current actors in a cell

		// create root actors map to store all actors with no parent
		std::unordered_map<std::string, Ref<Actor>> rootActors;

		// store all actors names in the scene 
		std::vector<std::string> allActorNames = sceneGraph->GetAllActorNames();

		// sort the names
		std::sort(allActorNames.begin(), allActorNames.end());

		for (const std::string& actorName : allActorNames) {
			Ref<Actor> actor = sceneGraph->GetActor(actorName);

			// if actor is a root actor, add it to the map
			if (actor && actor->isRootActor()) {
				rootActors.emplace(actorName, actor);
			}
		}
		///


		for (const auto& pair : rootActors) {
			DrawActorNode(pair.first, pair.second);
		}

		// TODO: right click popup menu, create new, remove, rename 
	}
	ImGui::End();
}

void HierarchyWindow::DrawActorNode(const std::string& actorName, Ref<Actor> actor)
{
	//
	std::unordered_map<std::string, Ref<Actor>> childActors = GetChildActors(actor.get());

	// imgui_demo.cpp Widgets/Tree Nodes/Advanced, with Selectable nodes
	const bool is_selected = sceneGraph->debugSelectedAssets.find(actorName) != sceneGraph->debugSelectedAssets.end();

	bool node_filter = filter.PassFilter(actorName.c_str());

	// show selection if node is selected
	bool show_selection = true;
	if (show_only_selected) {
		show_selection = is_selected || HasSelectedChild(actor.get());
	}

	// show if node appears in filter
	bool show_filter = node_filter || HasFilteredChild(actor.get());

	// draw node if it is selected and/or in filter
	if (!show_selection || !show_filter) {
		return;
	}

	// converting actor name to const char to create a unique ID for its node
	ImGui::PushID(actorName.c_str());

	// default flags for the the tree nodes
	ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth; // Standard opening mode as we are likely to want to add selection afterwards
	base_flags |= ImGuiTreeNodeFlags_NavLeftJumpsToParent; // Enable pressing left to jump to parent

	if (is_selected) {
		// set flag to selected
		base_flags |= ImGuiTreeNodeFlags_Selected;
	}

	if (childActors.empty()) {
		base_flags |= ImGuiTreeNodeFlags_Leaf;
	}

	bool node_open = ImGui::TreeNodeEx(actorName.c_str(), base_flags, "%s", actorName.c_str());

	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
		// using joel's raycast code for selection of actors in the window
		if (!ImGui::GetIO().KeyCtrl && !(is_selected)) { sceneGraph->debugSelectedAssets.clear(); }

		if (is_selected && ImGui::GetIO().KeyCtrl) { sceneGraph->debugSelectedAssets.erase(actorName); }

		else sceneGraph->debugSelectedAssets.emplace(actorName, actor);

	}

	if (node_open) {
		for (const auto& child : childActors) {
			DrawActorNode(child.first, child.second);
		}
		ImGui::TreePop();
	}

	ImGui::PopID();
}

bool HierarchyWindow::HasFilteredChild(Component* parent)
{
	std::unordered_map<std::string, Ref<Actor>> childActors = GetChildActors(parent);

	for (const auto& child : childActors) {
		if (filter.PassFilter(child.first.c_str())) { return true; } //!

		// recursive check to see if the child has children
		if (HasFilteredChild(child.second.get())) { return true; }
	}

	return false;
}

bool HierarchyWindow::HasSelectedChild(Component* parent)
{
	std::unordered_map<std::string, Ref<Actor>> childActors = GetChildActors(parent);

	for (const auto& child : childActors) {
		if (show_only_selected && sceneGraph->debugSelectedAssets.find(child.first) != sceneGraph->debugSelectedAssets.end()) { return true; } //==

		// recursive check to see if the child has children
		if (HasSelectedChild(child.second.get())) { return true; }
	}

	return false;
}

std::unordered_map<std::string, Ref<Actor>> HierarchyWindow::GetChildActors(Component* parent)
{
	/// switch to reading XML file after
	std::unordered_map<std::string, Ref<Actor>> childActors;

	// store all actors names in the scene 
	std::vector<std::string> allActorNames = sceneGraph->GetAllActorNames();

	// sort the names
	std::sort(allActorNames.begin(), allActorNames.end());

	for (const std::string& actorName : allActorNames) {
		Ref<Actor> actor = sceneGraph->GetActor(actorName);

		// actor is a child actor
		if (actor && actor->getParentActor() == parent) {
			childActors.emplace(actorName, actor);
		}
	}
	///

	return childActors;
}
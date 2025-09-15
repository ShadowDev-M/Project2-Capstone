#include "HierarchyWindow.h"

HierarchyWindow::HierarchyWindow(SceneGraph* sceneGraph_) : sceneGraph(sceneGraph_) {}

void HierarchyWindow::ShowHierarchyWindow(bool* pOpen)
{

	// Some of the stuff here will be changed after to just read off an XML file (like getting all the actors names and if they are parented/children)
	// just doing it without XML stuff for now since I don't fully understand it yet

	if (ImGui::Begin("Hierarchy", pOpen, ImGuiWindowFlags_MenuBar)) {

		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("Options")) {

				ImGui::Checkbox("Show Only Selected", &showOnlySelected);

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

		//
		if (ImGui::BeginPopupContextWindow("##RightClickHierarchyWindow")) {
			
			if (ImGui::MenuItem("Create Empty Actor")) {
				showAddActorDialog = true;
			}
			
			ImGui::EndPopup();
		}

		if (showAddActorDialog) {
			ImGui::OpenPopup("New Actor");
			showAddActorDialog = false;
		}

		// sets the placement and size of the dialog box
		const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x - 200, mainViewport->WorkPos.y - 200), ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Appearing);

		if (ImGui::BeginPopupModal("New Actor", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Enter New Actor's Name:");
			ImGui::InputText("##InputActorName", &newActorName);
			ImGui::Separator();

			if (ImGui::Button("Add Actor")) {
				Ref<Actor> newActor = std::make_shared<Actor>(nullptr, newActorName);
				newActor->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, 0.0f), Quaternion(0.0f, Vec3(0.0f, 0.0f, 0.0f)), Vec3(1.0f, 1.0f, 1.0f));
				
				// default shader
				newActor->AddComponent<ShaderComponent>(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Phong"));
				
				newActor->OnCreate();
				sceneGraph->AddActor(newActor);
				newActorName.clear();
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel")) {
				newActorName.clear();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}


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
	const bool isSelected = sceneGraph->debugSelectedAssets.find(actorName) != sceneGraph->debugSelectedAssets.end();

	bool nodeFilter = filter.PassFilter(actorName.c_str());

	// show selection if node is selected
	bool showSelection = true;
	if (showOnlySelected) {
		showSelection = isSelected || HasSelectedChild(actor.get());
	}

	// show if node appears in filter
	bool showFilter = nodeFilter || HasFilteredChild(actor.get());

	// draw node if it is selected and/or in filter
	if (!showSelection || !showFilter) {
		return;
	}

	// converting actor name to const char to create a unique ID for its node
	ImGui::PushID(actorName.c_str());

	// default flags for the the tree nodes
	ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth; // Standard opening mode as we are likely to want to add selection afterwards
	baseFlags |= ImGuiTreeNodeFlags_NavLeftJumpsToParent; // Enable pressing left to jump to parent

	if (isSelected) {
		// set flag to selected
		baseFlags |= ImGuiTreeNodeFlags_Selected;
	}

	if (childActors.empty()) {
		baseFlags |= ImGuiTreeNodeFlags_Leaf;
	}

	bool nodeOpen = ImGui::TreeNodeEx(actorName.c_str(), baseFlags, "%s", actorName.c_str());

	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
		// using joel's raycast code for selection of actors in the window
		if (!ImGui::GetIO().KeyCtrl && !(isSelected)) { sceneGraph->debugSelectedAssets.clear(); }

		if (isSelected && ImGui::GetIO().KeyCtrl) { sceneGraph->debugSelectedAssets.erase(actorName); }

		else sceneGraph->debugSelectedAssets.emplace(actorName, actor);

	}

	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::MenuItem("Delete")) {
			sceneGraph->GetActor(actorName)->DeleteComponent<
				Component>();
			sceneGraph->RemoveActor(actorName);
			sceneGraph->checkValidCamera();

		}
		ImGui::EndPopup();
	}

	if (nodeOpen) {
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
		if (showOnlySelected && sceneGraph->debugSelectedAssets.find(child.first) != sceneGraph->debugSelectedAssets.end()) { return true; }

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
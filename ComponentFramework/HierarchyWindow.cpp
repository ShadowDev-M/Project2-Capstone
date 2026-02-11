#include "pch.h"
#include "HierarchyWindow.h"
#include "EditorManager.h"

HierarchyWindow::HierarchyWindow(SceneGraph* sceneGraph_) : sceneGraph(sceneGraph_) {
	EditorManager::getInstance().RegisterWindow("Hierarchy", true);
}

void HierarchyWindow::ShowHierarchyWindow(bool* pOpen)
{
	// check editor manager for pending rename
	if (EditorManager::getInstance().HasPendingRename()) {
		auto [oldName, newName] = EditorManager::getInstance().ConsumePendingRename();
		sceneGraph->RenameActor(oldName, newName);
	}

	if (ImGui::Begin("Hierarchy", pOpen, ImGuiWindowFlags_MenuBar)) {

		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("Options")) {

				ImGui::Checkbox("Show Only Selected", &showOnlySelected);

				if (ImGui::MenuItem("Select All")) {
					std::vector<std::string> allActorNames = sceneGraph->GetAllActorNames();
					for (const auto& actorName : allActorNames) {
						Ref<Actor> actor = sceneGraph->GetActor(actorName);
						sceneGraph->debugSelectedAssets.emplace(actor->getId(), actor);
					}
				}

				if (ImGui::MenuItem("Clear All Selected")) {
					sceneGraph->debugSelectedAssets.clear();
				}


				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		filter.Draw("##HierarchyFilter", -1.0f);
		ImGui::Separator();

		// empty space right click
		if (ImGui::BeginPopupContextWindow("##RightClickHierarchyWindow")) {
			
			if (ImGui::MenuItem("Create Empty Actor")) {
				std::string actorName = "EmptyActor";
				std::string newActorName = GenerateDuplicateName(actorName);

				Ref<Actor> newActor = std::make_shared<Actor>(nullptr, newActorName);
				newActor->AddComponent<TransformComponent>(newActor.get(), Vec3(0.0f, 0.0f, 0.0f),
					Quaternion(1.0f, Vec3(0.0f, 0.0f, 0.0f)), Vec3(1.0f, 1.0f, 1.0f));
				newActor->OnCreate();
				sceneGraph->AddActor(newActor);
			}
			
			ImGui::EndPopup();
		}

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

		// draw the actual hierarchy tree
		for (const auto& pair : rootActors) {
			DrawActorNode(pair.first, pair.second);
		}

		// prepare the actor drag and drop
		if (const ImGuiPayload* payload = ImGui::GetDragDropPayload()) {
			
			if (payload->IsDataType("ACTOR_NODE") && ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {

				// check for empty window space
				if (!ImGui::IsAnyItemHovered()) {
					
					// so two options here: 
					// first, I have a button that appears when an actor is being dragging and use that to unparent, not fun but "shouldnt" break ImGui::Button("Unparent", ImVec2(-1, 0))
					// OR the fun approach, make it so the empty space below all the actors unparents (only issue with this is that if there are too many actors there won't be any empty space... but that wont happen surely)
					
					ImVec2 availSpace = ImGui::GetContentRegionAvail();
					if (availSpace.y > 0) {
						ImGui::Dummy(availSpace);

						if (ImGui::BeginDragDropTarget()) {
							const ImGuiPayload* acceptedPayload = ImGui::AcceptDragDropPayload("ACTOR_NODE");

							if (acceptedPayload) {
								const char* draggedActorName = static_cast<const char*>(acceptedPayload->Data);

								Ref<Actor> draggedActor = sceneGraph->GetActor(draggedActorName);

								if (draggedActor && !draggedActor->isRootActor()) {

									if (sceneGraph->debugSelectedAssets.size() != 0) {
										for (auto& obj : sceneGraph->debugSelectedAssets) {
											obj.second->unparent();
										}
									}
									
								}


							}

							ImGui::EndDragDropTarget();
						}
					}


				}
			}
		}

		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && !ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
				sceneGraph->debugSelectedAssets.clear();
			}
		}

	}
	ImGui::End();
}

void HierarchyWindow::DrawActorNode(const std::string& actorName_, Ref<Actor> actor_)
{
	std::unordered_map<std::string, Ref<Actor>> childActors = GetChildActors(actor_.get());

	// imgui_demo.cpp Widgets/Tree Nodes/Advanced, with Selectable nodes
	const bool isSelected = sceneGraph->debugSelectedAssets.find(actor_->getId()) != sceneGraph->debugSelectedAssets.end();

	bool nodeFilter = filter.PassFilter(actorName_.c_str());

	// show selection if node is selected
	bool showSelection = true;
	if (showOnlySelected) {
		showSelection = isSelected || HasSelectedChild(actor_.get());
	}

	// show if node appears in filter
	bool showFilter = nodeFilter || HasFilteredChild(actor_.get());

	// draw node if it is selected and/or in filter
	if (!showSelection || !showFilter) {
		return;
	}

	// default flags for the the tree nodes
	ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | 
								   ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NavLeftJumpsToParent;

	if (isSelected) {
		// set flag to selected
		baseFlags |= ImGuiTreeNodeFlags_Selected;
	}

	if (childActors.empty()) {
		baseFlags |= ImGuiTreeNodeFlags_Leaf;
	}

	// converting actor name to const char to create a unique ID for its node
	ImGui::PushID(actorName_.c_str());

	bool nodeOpen = ImGui::TreeNodeEx(actorName_.c_str(), baseFlags, "%s", actorName_.c_str());

	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
		// using joel's raycast code for selection of actors in the window
		if (!ImGui::GetIO().KeyCtrl && !isSelected) {
			sceneGraph->debugSelectedAssets.clear();
		}

		if (isSelected && ImGui::GetIO().KeyCtrl) {
			sceneGraph->debugSelectedAssets.erase(actor_->getId());
		}
		else {
			sceneGraph->debugSelectedAssets.emplace(actor_->getId(), actor_);
		}
	}

	HandleDragDrop(actorName_, actor_);

	if (ImGui::BeginPopupContextItem("##ActorContext")) {
		if (ImGui::MenuItem("Duplicate")) {
			DuplicateActor(actor_);
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Delete")) {
			sceneGraph->RemoveLight(sceneGraph->GetActor(actorName_));
			sceneGraph->GetActor(actorName_)->DeleteComponent<LightComponent>();
			//sceneGraph->GetActor(actorName_)->DeleteComponent<Component>();

			sceneGraph->RemoveActor(actorName_);
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

void HierarchyWindow::DuplicateActor(Ref<Actor> original_) {
	if (!original_) return;

	std::unordered_map<std::string, Ref<Actor>> childActors = GetChildActors(original_.get());
	if (original_->getParentActor() == nullptr) {
		std::string newName = GenerateDuplicateName(original_->getActorName());

		Ref<Actor> parentActor = DeepCopyActor(newName, original_);
		parentActor->OnCreate();
		sceneGraph->AddActor(parentActor);

		if (childActors.size() != 0) {
			for (const auto& child : childActors) {
				newName = GenerateDuplicateName(child.first);
				
				Ref<Actor> childActor = DeepCopyActor(newName, child.second);
				childActor->setParentActor(parentActor.get());
				childActor->OnCreate();
				sceneGraph->AddActor(childActor);
			}
		}
	}
	else if (original_->getParentActor() != nullptr) {
		std::string newName = GenerateDuplicateName(original_->getActorName());

		Ref<Actor> parentActor = DeepCopyActor(newName, original_);
		parentActor->OnCreate();
		sceneGraph->AddActor(parentActor);
	}
}

Ref<Actor> HierarchyWindow::DeepCopyActor(const std::string& newName_, Ref<Actor> original_) {
	if (!original_) return nullptr;

	Ref<Actor> copy = std::make_shared<Actor>(original_->getParentActor(), newName_);

	if (auto transform = original_->GetComponent<TransformComponent>()) {
		copy->AddComponent<TransformComponent>(copy.get(),
			transform->GetPosition(),
			transform->GetQuaternion(),
			transform->GetScale());
	}

	if (auto mesh = original_->GetComponent<MeshComponent>()) {
		copy->AddComponent<MeshComponent>(mesh);
	}

	if (auto material = original_->GetComponent<MaterialComponent>()) {
		copy->AddComponent<MaterialComponent>(material);
	}

	if (auto shader = original_->GetComponent<ShaderComponent>()) {
		copy->AddComponent<ShaderComponent>(shader);
	}

	if (auto camera = original_->GetComponent<CameraComponent>()) {
		copy->AddComponent<CameraComponent>(copy);
	}

	if (auto light = original_->GetComponent<LightComponent>()) {
		copy->AddComponent<LightComponent>(nullptr, light->getType(), light->getSpec(), light->getDiff(), light->getIntensity());
	}

	return copy;
}

std::string HierarchyWindow::GenerateDuplicateName(const std::string& originalName) {
	std::string baseName = originalName;
	int counter = 1;

	// this whole thing just makes it so that if you are duplicating an actor like Cube_3D, it wont do Cube_3D_1D and it'll make sure its Cube_4D
	
	// find the start of the duplicated name
	size_t start = originalName.find_last_of('_');
	
	// make sure that its not the end of the string
	if (start != std::string::npos) {

		// find the end of the duplicated name
		size_t end = originalName.find_last_of('D');
		
		// get whats inbetween
		if (end == originalName.length() - 1) {
			baseName = originalName.substr(0, start);
		}
	}

	std::string newName;
	do {
		newName = baseName + "_" + std::to_string(counter++) + "D"; // can't end or start with a special character, XML breaks so no ( )
	} while (sceneGraph->GetActor(newName) != nullptr);

	return newName;
}

void HierarchyWindow::HandleDragDrop(const std::string& actorName_, Ref<Actor> actor_) {
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
		
		// set the payload to carry the actor's name
		ImGui::SetDragDropPayload("ACTOR_NODE", actorName_.c_str(), actorName_.length() + 1);

		//string for all selected actors
		std::string draggingStr = "Dragging:";
		for (auto& obj : sceneGraph->debugSelectedAssets) {
			draggingStr += " " + obj.second->getActorName() + ",";

		}
		//Remove last comma from str
		draggingStr.pop_back();

		// show the actors names while dragging
		ImGui::Text(draggingStr.c_str());

		ImGui::EndDragDropSource();
	}

	if (ImGui::BeginDragDropTarget()) {
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ACTOR_NODE");

		if (payload) {
			// store payload data
			const char* draggedActorName = static_cast<const char*>(payload->Data);

			// make sure that a parent cant parent itself
			if (draggedActorName != actorName_) {
				Ref<Actor> draggedActor = sceneGraph->GetActor(draggedActorName);

				// check to make sure theres no circular dependency (don't parent an actor to one of its own children)
				bool wouldCreateCycle = false;

				//check all selected actors for circular 

				Actor* checkParent = actor_.get();
				while (checkParent != nullptr) {
					if (sceneGraph->debugSelectedAssets.find(checkParent->getId()) != sceneGraph->debugSelectedAssets.end()) {
						wouldCreateCycle = true;
						break;
					}
					checkParent = checkParent->getParentActor();
				}
			

				// wont create cycle, so its okay to parent
				if (!wouldCreateCycle && draggedActor) {

					//place all selected assets into the parent actor
					for (auto& obj : sceneGraph->debugSelectedAssets) 
					{
						//You can't place itself into itself, skip this one.
						if (obj.second == actor_) continue;

						obj.second->setParentActor(actor_.get());
					}
				}
				else if (wouldCreateCycle) {
					Debug::Warning("Cannot parent actor to its own child", __FILE__, __LINE__);
				}
			}
		}

		ImGui::EndDragDropTarget();
	}
}

bool HierarchyWindow::HasFilteredChild(Component* parent)
{
	std::unordered_map<std::string, Ref<Actor>> childActors = GetChildActors(parent);

	for (const auto& child : childActors) {
		if (filter.PassFilter(child.first.c_str())) { return true; } 

		// recursive check to see if the child has children
		if (HasFilteredChild(child.second.get())) { return true; }
	}

	return false;
}

bool HierarchyWindow::HasSelectedChild(Component* parent)
{
	std::unordered_map<std::string, Ref<Actor>> childActors = GetChildActors(parent);

	for (const auto& child : childActors) {
		if (sceneGraph->debugSelectedAssets.find(child.second->getId()) != sceneGraph->debugSelectedAssets.end()) { return true; }

		// recursive check to see if the child has children
		if (HasSelectedChild(child.second.get())) { return true; }
	}

	return false;
}

std::unordered_map<std::string, Ref<Actor>> HierarchyWindow::GetChildActors(Component* parent)
{
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
	
	return childActors;
}
#include "DockingWindow.h"
#include "InputManager.h"
#include "EditorManager.h"


DockingWindow::DockingWindow(SceneGraph* sceneGraph_) : sceneGraph(sceneGraph_) {
    EditorManager::getInstance().RegisterWindow("Scene", true);
}

void DockingWindow::ShowDockingWindow(bool* pOpen)
{

    
	if (ImGui::Begin("Scene", pOpen, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_MenuBar)) {
		
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Camera")) {                
                if (ImGui::MenuItem("Change Camera to Default ##MenuItem", "Ctrl+M")) {
                    if (sceneGraph) {
                        sceneGraph->useDebugCamera();
                    }
                }
                
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Draw Modes")) {
                if (ImGui::MenuItem("Wireframe")) {
                    sceneGraph->SetDrawMode(GL_LINE);
                }
                if (ImGui::MenuItem("Shaded Draw Mode")) {
                    sceneGraph->SetDrawMode(GL_FILL);
                }

                ImGui::EndMenu();
            }
               
            if (ImGui::BeginMenu("Grid")) {
                

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Tools")) {
                // slider for increasing stud multiplier (in-scene movement with wasd)
                float sliderMulti = InputManager::getInstance().GetStudMultiplier();
                ImGui::Text("Stud Multi");
                ImGui::SameLine();
                if (ImGui::SliderFloat("##StudSlider", &sliderMulti, 0.0f, 10.0f, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
                    InputManager::getInstance().SetStudMultiplier(sliderMulti);
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

		/*ImVec2 size = ImGui::GetContentRegionAvail();
		ImGui::Image((ImTextureID)(intptr_t)SceneGraph::getInstance().dockingFBO, size, ImVec2(0, 1), ImVec2(1, 0));
       */
        
        int w, h;

        w = SceneGraph::SCENEWIDTH;
        h = SceneGraph::SCENEHEIGHT;

        //SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &w, &h);

        float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImVec2 scaledTexture;

        // Calculate scaled dimensions based on aspect ratio
        if (windowSize.x / aspectRatio <= windowSize.y)
        {
            scaledTexture.x = windowSize.x;
            scaledTexture.y = windowSize.x / aspectRatio;
        }
        else
        {
            scaledTexture.y = windowSize.y;
            scaledTexture.x = windowSize.y * aspectRatio;
        }

        // Center the image if there's empty space
        ImVec2 imagePos = ImVec2((windowSize.x - scaledTexture.x) * 0.5f,( (windowSize.y - scaledTexture.y) * 0.5f) + ImGui::GetFrameHeight());

        // Display the image with calculated dimensions
        ImGui::SetCursorPos(imagePos);
        ImGui::Image((void*)(intptr_t)SceneGraph::getInstance().dockingTexture, scaledTexture, ImVec2(0, 1), ImVec2(1, 0));
        
        if (ImGui::BeginDragDropTarget()) {

            dropAssetOnScene<MaterialComponent>();
            dropAssetOnScene<ShaderComponent>();
            dropAssetOnScene<MeshComponent>();

            
            ImGui::EndDragDropTarget();
        }


        ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;

        InputManager::getInstance().getMouseMap()->frameHeight = ImGui::GetFrameHeight();

        InputManager::getInstance().getMouseMap()->dockingPos = ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetFrameHeight());
        InputManager::getInstance().getMouseMap()->dockingSize = ImGui::GetWindowSize();

        InputManager::getInstance().getMouseMap()->dockingHovered = ImGui::IsWindowHovered();
        InputManager::getInstance().getMouseMap()->dockingClicked = InputManager::getInstance().getMouseMap()->dockingHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);


        InputManager::getInstance().updateDockingFocused(ImGui::IsWindowFocused());



	}
	ImGui::End();
}

bool getClickInteraction() {
    if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        return true;
    }
}

template<typename ComponentTemplate>
void DockingWindow::dropAssetOnScene() {

    std::string dataType;
    if (std::is_same_v<ComponentTemplate, MaterialComponent>) dataType = "MATERIAL_ASSET";  
    else if (std::is_same_v<ComponentTemplate, ShaderComponent>) dataType = "SHADER_ASSET";
    else if (std::is_same_v<ComponentTemplate, MeshComponent>) dataType = "MESH_ASSET";

    const char* dataTypeC_Str = dataType.c_str();

    if (ImGui::GetDragDropPayload()->IsDataType(dataTypeC_Str)) {
        int mx, my;
        SDL_GetMouseState(&mx, &my);

        Ref<Actor> payloadActor = sceneGraph->pickColour(mx, my);

        
        const char* droppedAssetName = static_cast<const char*>(ImGui::GetDragDropPayload()->Data);

        static Ref<Component> originalComponent;
        static Ref<Actor> originalActor;
       
        //If its a MESH, make sure to colour picked based on the ORIGINAL, so restore it and redo the picking
        if (std::is_same_v<ComponentTemplate, MeshComponent> && originalActor) {
            originalActor->ReplaceComponent<ComponentTemplate>(std::dynamic_pointer_cast<ComponentTemplate>(originalComponent));
            originalActor = 0;
            originalComponent = 0;
            payloadActor = sceneGraph->pickColour(mx, my);

        }

        //There's a picked object but its NOT the original actor, so restore the original to prepare for the new one
        if (payloadActor && originalActor && payloadActor != originalActor) {
            originalActor->ReplaceComponent<ComponentTemplate>(std::dynamic_pointer_cast<ComponentTemplate>(originalComponent));
            originalActor = 0;
            originalComponent = 0;

            

        }


        //Picked actor but no original, so you're good to save a new original 
        if (payloadActor && !originalActor) {
            originalActor = payloadActor;
            originalComponent = originalActor->GetComponent<ComponentTemplate>();




        }
        // get a reference to the asset that has been dropped
        Ref<ComponentTemplate> newAsset = AssetManager::getInstance().GetAsset<ComponentTemplate>(droppedAssetName);

        // get the actor
        if (newAsset) {
            //get the mouse and see if the mouse can pick an object when it drops the material


            if (payloadActor) {
                if (payloadActor->GetComponent<ComponentTemplate>()) {
                    payloadActor->ReplaceComponent<ComponentTemplate>(newAsset);
                }

            }
        }

        
        //dropped
        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(dataTypeC_Str);

        if (payload) {
            
            //wipe the original as we want the temp component to be permanent
            originalActor = 0;
            originalComponent = 0;
            
        }

        //If no picked actor but originalActor is saving its hovered component, restore it
        if (!payloadActor && originalActor) { 
            originalActor->ReplaceComponent<ComponentTemplate>(std::dynamic_pointer_cast<ComponentTemplate>(originalComponent));
            originalActor = 0;
            originalComponent = 0;

        }

    }
}
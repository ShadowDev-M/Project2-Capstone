#include "pch.h"
#include "SceneWindow.h"
#include "InputManager.h"
#include "EditorManager.h"

SceneWindow::SceneWindow(SceneGraph* sceneGraph_) : sceneGraph(sceneGraph_) {
    EditorManager::getInstance().RegisterWindow("Scene", true);
}

void SceneWindow::ShowSceneWindow(bool* pOpen)
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

            if (ImGui::BeginMenu("Gizmo")) {
                if (ImGui::MenuItem("Translate", "W", currentGizmoOperation == ImGuizmo::TRANSLATE)) {
                    currentGizmoOperation = ImGuizmo::TRANSLATE;
                }
                if (ImGui::MenuItem("Rotate", "E", currentGizmoOperation == ImGuizmo::ROTATE)) {
                    currentGizmoOperation = ImGuizmo::ROTATE;
                }
                if (ImGui::MenuItem("Scale", "R", currentGizmoOperation == ImGuizmo::SCALE)) {
                    currentGizmoOperation = ImGuizmo::SCALE;
                }

                ImGui::Separator();

                if (ImGui::MenuItem("World Space", nullptr, currentGizmoMode == ImGuizmo::WORLD)) {
                    currentGizmoMode = ImGuizmo::WORLD;
                }
                if (ImGui::MenuItem("Local Space", nullptr, currentGizmoMode == ImGuizmo::LOCAL)) {
                    currentGizmoMode = ImGuizmo::LOCAL;
                }

                ImGui::Separator();

                ImGui::Checkbox("Use Snap", &useGizmoSnap);
                if (useGizmoSnap) {
                    ImGui::DragFloat3("Snap Values", gizmoSnapValues, 0.1f, 0.1f, 10.0f);
                }

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

        DrawGizmos(scaledTexture, imagePos);

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

void SceneWindow::DrawGizmos(ImVec2 scaledTexture_, ImVec2 imagePos_) {
    
    // window/viewport size calculations to pass onto imguizmo
    ImVec2 windowPos = ImGui::GetWindowPos();
    float viewportX = windowPos.x + imagePos_.x;
    float viewportY = windowPos.y + imagePos_.y;

    // an extra check to make sure its drawing on this window
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(viewportX, viewportY, scaledTexture_.x, scaledTexture_.y);

    // make sure only one actor is selected
    if (sceneGraph->debugSelectedAssets.size() == 1) {
        auto selectedActor = sceneGraph->debugSelectedAssets.begin()->second;

        Ref<CameraComponent> camera = sceneGraph->getUsedCamera();

        if (camera && selectedActor->GetComponent<TransformComponent>()) {
            Matrix4 view = camera->GetViewMatrix();
            Matrix4 projection = camera->GetProjectionMatrix();
            Matrix4 model = selectedActor->GetModelMatrix();

            // imguizmo takes float arrays, so gotta convert
            float viewMatrix[16];
            float projectionMatrix[16];
            float modelmatrix[16];

            ConvertMat4toFloatArray(view, viewMatrix);
            ConvertMat4toFloatArray(projection, projectionMatrix);
            ConvertMat4toFloatArray(model, modelmatrix);

            // 
            //float originalMatrix[16];
            //std::memcpy(originalMatrix, modelmatrix, sizeof(float) * 16);

            bool isManipulating = ImGuizmo::Manipulate(viewMatrix, projectionMatrix, currentGizmoOperation, currentGizmoMode, modelmatrix, nullptr, useGizmoSnap ? gizmoSnapValues : nullptr);

            // the actual imguizmo code to get the gizmos up and running
            if (isManipulating && ImGuizmo::IsUsing()) {
                // decomposing the modelmatrix to get the translation, rotation, and scale
                float translation[3], rotation[3], scale[3];
                ImGuizmo::DecomposeMatrixToComponents(modelmatrix, translation, rotation, scale);

                // update postion, rotation, and scale according to how the gizmo was moved (world space)
                Vec3 worldPos(translation[0], translation[1], translation[2]);
                Euler eulerRotation(rotation[0], rotation[1], rotation[2]);
                Quaternion worldRotation = QMath::toQuaternion(eulerRotation);
                Vec3 worldScale(scale[0], scale[1], scale[2]);

                Ref<TransformComponent> transform = selectedActor->GetComponent<TransformComponent>();

                // when decomposing the matrix with imguizmo, it returns values in world space
                // however, child actors do not operate in world space
                // they operate in a local space relative to the parent actor
                // so we have to convert
                if (!selectedActor->isRootActor() && selectedActor->getParentActor() != nullptr) {
                    // converting world space parent matrix to local space
                    Matrix4 parentModelMatrix = selectedActor->getParentActor()->GetModelMatrix();
                    Matrix4 inverseParent = MMath::inverse(parentModelMatrix);

                    // inverse * T * R * S
                    Matrix4 localMatrix = inverseParent * MMath::translate(worldPos) * MMath::toMatrix4(worldRotation) * MMath::scale(worldScale);
                    
                    // decomposing the local matrix to get the local translation, rotation, and scale
                    ConvertMat4toFloatArray(localMatrix, modelmatrix);
                    float localTranslation[3], localRotation[3], _localScale[3];
                    ImGuizmo::DecomposeMatrixToComponents(modelmatrix, localTranslation, localRotation, _localScale);

                    // update postion, rotation, and scale according to how the gizmo was moved (local space)
                    Vec3 localPos(localTranslation[0], localTranslation[1], localTranslation[2]);
                    Euler localEulerRotation(localRotation[0], localRotation[1], localRotation[2]);
                    Quaternion localRot = QMath::toQuaternion(localEulerRotation);
                    Vec3 localScale(_localScale[0], _localScale[1], _localScale[2]);
                    
                    transform->SetTransform(localPos, localRot, localScale);
                }
                else {
                    transform->SetTransform(worldPos, worldRotation, worldScale);
                }
            }
        }
    }
}

void SceneWindow::ConvertMat4toFloatArray(const Matrix4& matrix_, float* array_)
{
    for (int i = 0; i < 16; i++) {
        array_[i] = matrix_[i];
    }
}

void SceneWindow::ConvertFloatArraytoMat4(Matrix4& matrix_, float* array_)
{
    for (int i = 0; i < 16; i++) {
        matrix_[i] = array_[i];
    }
}

bool getClickInteraction() {
    if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        return true;
    }
    return false;
}

template<typename ComponentTemplate>
void SceneWindow::dropAssetOnScene() {

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
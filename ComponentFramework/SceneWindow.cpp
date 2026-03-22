#include "pch.h"
#include "SceneWindow.h"
#include "InputManager.h"
#include "EditorManager.h"
#include "ScreenManager.h"
#include "FBOManager.h"
#include "Renderer.h"
#include "GameWindow.h"

SceneWindow::SceneWindow(SceneGraph* sceneGraph_) : sceneGraph(sceneGraph_) {
    EditorManager::getInstance().RegisterWindow("Scene", true);
}

void SceneWindow::ShowSceneWindow(bool* pOpen)
{
	if (ImGui::Begin("Scene", pOpen, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_MenuBar)) {
		
        EditorCamera& editorCam = EditorManager::getInstance().getEditorCamera();

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Tools")) {
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

            if (ImGui::BeginMenu("Grid")) {

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Draw Modes")) {
                if (ImGui::MenuItem("Wireframe")) {
                    Renderer::getInstance().SetDrawMode(GL_LINE);
                }
                if (ImGui::MenuItem("Shaded Draw Mode")) {
                    Renderer::getInstance().SetDrawMode(GL_FILL);
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Camera View")) {                
                bool is2D = (editorCam.GetMode() == Mode::Mode2D);
                if (ImGui::MenuItem("2D Mode", nullptr, is2D)) {
                    if (!is2D) {
                        std::vector<Vec3> positions;
                        for (const auto& [id, actor] : sceneGraph->getAllActors()) {
                            Matrix4 modelMatrix = actor->GetModelMatrix();
                            positions.push_back(modelMatrix.getColumn(Matrix4::Colunm::three));
                        }
                        editorCam.EnterMode2D(positions);
                    }
                    else {
                        editorCam.SetMode(Mode::Mode3D);
                    }
                }

                bool isOrtho = editorCam.isOrtho() && editorCam.GetMode() != Mode::Mode2D;

                if (editorCam.GetMode() == Mode::Mode3D) {
                    if (ImGui::MenuItem("Orthographic", nullptr, isOrtho)) {
                        editorCam.SetIsOrtho(!isOrtho);
                    }
                }

                ImGui::Separator();
                
                if (ImGui::MenuItem("Front")) { editorCam.SetOrientation(0.0f, 0.0f); }
                if (ImGui::MenuItem("Back")) { editorCam.SetOrientation(180.0f, 0.0f); }
                if (ImGui::MenuItem("Right")) { editorCam.SetOrientation(90.0f, 0.0f); }
                if (ImGui::MenuItem("Left")) { editorCam.SetOrientation(-90.0f, 0.0f); }
                if (ImGui::MenuItem("Top")) { editorCam.SetOrientation(0.0f, 89.0f); }
                if (ImGui::MenuItem("Bottom")) { editorCam.SetOrientation(0.0f, -89.0f); }
                
                ImGui::Separator();

                if (ImGui::Button("Reset View")) editorCam.ResetView();

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Camera Settings")) {
                
                float fov = editorCam.GetFOV();
                float nearClipPlane = editorCam.GetNearClip(), farClipPlane = editorCam.GetFarClip();
                float orthoSize = editorCam.GetOrthoSize();
                float speed = editorCam.GetCameraSpeed();
                float speedMin = editorCam.GetSpeedMin();
                float speedMax = editorCam.GetSpeedMax();

                int fovInt = static_cast<int>(fov);

                if (!editorCam.isOrtho()) {
                    ImGui::Text("FOV");
                    ImGui::SameLine(100);
                    ImGui::SetNextItemWidth(150);

                    if (ImGui::SliderInt("##FOVSlider", &fovInt, 1, 120, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
                        editorCam.SetFOV((float)fovInt);
                    }
                }
                ImGui::ActiveItemLockMousePos();

                if (editorCam.isOrtho()) {
                    ImGui::Text("Size");
                    ImGui::SameLine(100);
                    ImGui::SetNextItemWidth(150);

                    if (ImGui::SliderFloat("##OrthoSizeSlider", &orthoSize, 0.0f, 100.0f, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
                        editorCam.SetOrthoSize(orthoSize);
                    }
                }
                ImGui::ActiveItemLockMousePos();

                ImGui::Text("Clipping Planes");

                ImGui::Text("Near");
                ImGui::SameLine(100);
                ImGui::SetNextItemWidth(150);
                if (ImGui::DragFloat("##NearDrag", &nearClipPlane, 1.0f, 0.0f, 100.0f, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
                    editorCam.SetNearClip(nearClipPlane);
                }
                ImGui::ActiveItemLockMousePos();

                ImGui::Text("Far");
                ImGui::SameLine(100);
                ImGui::SetNextItemWidth(150);
                if (ImGui::DragFloat("##FarDrag", &farClipPlane, 1.0f, 0.0f, 1000.0f, nullptr, ImGuiSliderFlags_AlwaysClamp)) {
                    editorCam.SetFarClip(farClipPlane);
                }
                ImGui::ActiveItemLockMousePos();

                ImGui::Text("Navigation");

                ImGui::Text("Camera Speed");
                ImGui::SameLine(100);
                ImGui::SetNextItemWidth(150);
                if (ImGui::DragFloat("##CamSpeed", &speed, 0.01f, editorCam.GetSpeedMin(), editorCam.GetSpeedMax(), "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
                    editorCam.SetCameraSpeed(speed);
                }
                ImGui::ActiveItemLockMousePos();

                ImGui::Text("Min");
                ImGui::SameLine(100);
                ImGui::SetNextItemWidth(150);
                if (ImGui::DragFloat("##MinSpeed", &speedMin, 0.01f, 0.0001f, editorCam.GetSpeedMax(), "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
                    editorCam.SetSpeedMin(speedMin);
                }
                ImGui::ActiveItemLockMousePos();

                ImGui::Text("Max");
                ImGui::SameLine(100);
                ImGui::SetNextItemWidth(150);
                if (ImGui::DragFloat("##MaxSpeed", &speedMax, 0.01f, editorCam.GetSpeedMin(), 10000.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
                    editorCam.SetSpeedMax(speedMax);
                }
                ImGui::ActiveItemLockMousePos();

                ImGui::Separator();

                if (ImGui::Button("Reset Settings")) editorCam.ResetSettings();

                ImGui::EndMenu();
            }

            ImGui::MenuItem("Gizmos", nullptr, &showDebugGizmos);

            ImGui::EndMenuBar();
        }
        Renderer::getInstance().showSceneGizmos = showDebugGizmos;

        // getting the full avaiable space of the window
        ImVec2 availSize = ImGui::GetContentRegionAvail();
        int w = std::max(1, (int)availSize.x);
        int h = std::max(1, (int)availSize.y);

        // filling out the entire window with the fbo
        FBOData& sceneFBO = FBOManager::getInstance().getFBO(FBO::Scene);
        if (sceneFBO.width != w || sceneFBO.height != h) {
            FBOManager::getInstance().OnResize(FBO::Scene, w, h);
        }

        // Center the image if there's empty space
        ImVec2 imagePos = ImGui::GetCursorPos();

        // Display the image with calculated dimensions;
        ImGui::Image((void*)(intptr_t)sceneFBO.texture, availSize, ImVec2(0, 1), ImVec2(1, 0));
        
        // passing window variables
        InputManager::getInstance().getMouseMap()->sceneFrame = ImGui::GetFrameHeight();
        InputManager::getInstance().getMouseMap()->scenePos = ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetFrameHeight());
        InputManager::getInstance().getMouseMap()->sceneSize = ImGui::GetWindowSize();
        InputManager::getInstance().getMouseMap()->sceneHovered = ImGui::IsWindowHovered();
        InputManager::getInstance().getMouseMap()->sceneClicked = InputManager::getInstance().getMouseMap()->sceneHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
        InputManager::getInstance().updateWindowFocused(ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows));

        // for editor camera controls
        EditorManager::getInstance().getEditorCamera().Update(ImGui::GetIO().DeltaTime, ImGui::IsWindowHovered());
        
        if (ImGui::BeginDragDropTarget()) {
            dropAssetOnScene<MaterialComponent>();
            dropAssetOnScene<ShaderComponent>();
            dropAssetOnScene<MeshComponent>();
            ImGui::EndDragDropTarget();
        }

        DrawGizmos(availSize, imagePos);

        if (!editorCam.isRMBHeld()) {
            if (ImGui::IsKeyPressed(ImGuiKey_W)) currentGizmoOperation = ImGuizmo::TRANSLATE;
            if (ImGui::IsKeyPressed(ImGuiKey_E)) currentGizmoOperation = ImGuizmo::ROTATE;
            if (ImGui::IsKeyPressed(ImGuiKey_R)) currentGizmoOperation = ImGuizmo::SCALE;

            if (ImGui::IsKeyPressed(ImGuiKey_F) && !sceneGraph->debugSelectedAssets.empty()) {
                uint32_t lastID = EditorManager::getInstance().GetLastSelected();
                if (sceneGraph->debugSelectedAssets.find(lastID) == sceneGraph->debugSelectedAssets.end()) {
                    lastID = sceneGraph->debugSelectedAssets.begin()->first;
                }
                Ref<Actor> lastActor = sceneGraph->debugSelectedAssets.at(lastID);

                editorCam.FrameTarget(lastActor->GetModelMatrix().getColumn(Matrix4::Colunm(3)));
                //ImGui::SetWindowFocus("Scene");
            }
        }

        if (editorCam.GetShowSpeedPopup()) {
            float alpha = editorCam.GetSpeedPopupAlpha(); 
            ImGui::SetCursorPos(ImVec2(10.0f, ImGui::GetFrameHeight() + 34.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, alpha));
            ImGui::Text("Speed: %.2f", editorCam.GetCameraSpeed());
            ImGui::PopStyleColor();
        }
	}
	ImGui::End();
}

void SceneWindow::DrawGizmos(ImVec2 scaledTexture_, ImVec2 imagePos_) {
    if (sceneGraph->debugSelectedAssets.empty()) return;

    EditorCamera& editorCam = EditorManager::getInstance().getEditorCamera();

    // window/viewport size calculations to pass onto imguizmo
    ImVec2 windowPos = ImGui::GetWindowPos();
    float viewportX = windowPos.x + imagePos_.x;
    float viewportY = windowPos.y + imagePos_.y;

    // an extra check to make sure its drawing on this window and making sure it works in ortho view
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(viewportX, viewportY, scaledTexture_.x, scaledTexture_.y);
    ImGuizmo::SetOrthographic(editorCam.isOrtho());

    // getting the last selected actor (gizmo shows on last selected but affects all selected)
    uint32_t lastID = EditorManager::getInstance().GetLastSelected();
    if (sceneGraph->debugSelectedAssets.find(lastID) == sceneGraph->debugSelectedAssets.end()) {
        lastID = sceneGraph->debugSelectedAssets.begin()->first;
    }
    Ref<Actor> lastActor = sceneGraph->debugSelectedAssets.at(lastID);

    if (!lastActor->GetComponent<TransformComponent>()) return;

    // saving the previous selected actors id, pos, rotation, and scale
    // this might be too overcomplicated but its the easiest way of just making sure the proper translations get applied accordingly
    struct PreviousActor { Vec3 position; Quaternion rotation; Vec3 scale; };
    bool wasUsing = false;
    uint32_t previousID = 0;
    std::unordered_map<uint32_t, PreviousActor> previousTransform;
    
    bool isUsing = ImGuizmo::IsUsing();
    if (isUsing && !wasUsing) {
        previousTransform.clear();
        previousID = lastID;
        for (auto& [id, actor] : sceneGraph->debugSelectedAssets) {
            Ref<TransformComponent> TC = actor->GetComponent<TransformComponent>();
            if (TC) previousTransform[id] = { TC->GetPosition(), TC->GetOrientation(), TC->GetScale() };
        }
    }
    wasUsing = isUsing;

    Matrix4 view = editorCam.GetViewMatrix();
    Matrix4 projection = editorCam.GetProjectionMatrix();
    Matrix4 model = lastActor->GetModelMatrix();

    // imguizmo takes float arrays, so gotta convert
    float viewMatrix[16];
    float projectionMatrix[16];
    float modelmatrix[16];
    ConvertMat4toFloatArray(view, viewMatrix);
    ConvertMat4toFloatArray(projection, projectionMatrix);
    ConvertMat4toFloatArray(model, modelmatrix);

    // makes sdl capture mouse so using gizmos doesnt get stopped when hitting another window
    SDL_CaptureMouse(ImGuizmo::IsUsing() ? SDL_TRUE : SDL_FALSE);
    
    bool isManipulating = ImGuizmo::Manipulate(viewMatrix, projectionMatrix, currentGizmoOperation, currentGizmoMode, modelmatrix, nullptr, useGizmoSnap ? gizmoSnapValues : nullptr);
    
    if (!isManipulating || !isUsing) return;

    // decomposing the modelmatrix to get the translation, rotation, and scale
    float translation[3], rotation[3], scale[3];
    ImGuizmo::DecomposeMatrixToComponents(modelmatrix, translation, rotation, scale);
    
    // update postion, rotation, and scale according to how the gizmo was moved (world space)
    Vec3 lastPos(translation[0], translation[1], translation[2]);
    Euler lastEulerRotation(rotation[0], rotation[1], rotation[2]);
    Quaternion lastRotation = QMath::toQuaternion(lastEulerRotation);
    Vec3 lastScale(scale[0], scale[1], scale[2]);
    
    // getting the delta from previous actor
    Vec3 deltaPos = lastPos - previousTransform[lastID].position;
    Quaternion deltaRot = lastRotation * QMath::inverse(previousTransform[lastID].rotation);
    Vec3 deltaScale = Vec3( // divide by error check
        previousTransform[lastID].scale.x > 0.0001f ? lastScale.x / previousTransform[lastID].scale.x : 1.0f,
        previousTransform[lastID].scale.y > 0.0001f ? lastScale.y / previousTransform[lastID].scale.y : 1.0f, 
        previousTransform[lastID].scale.z > 0.0001f ? lastScale.z / previousTransform[lastID].scale.z : 1.0f);

    for (auto& [id, actor] : sceneGraph->debugSelectedAssets) {
        Ref<TransformComponent> TC = actor->GetComponent<TransformComponent>();
        auto it = previousTransform.find(id);
        if (!TC || it == previousTransform.end()) continue;

        // root actors
        const PreviousActor& prevActor = it->second;
        Vec3 worldPos = prevActor.position + deltaPos;
        Quaternion worldRotation = prevActor.rotation * deltaRot;
        Vec3 worldScale = Vec3(
            prevActor.scale.x * deltaScale.x,
            prevActor.scale.y * deltaScale.y,
            prevActor.scale.z * deltaScale.z);

        // when decomposing the matrix with imguizmo, it returns values in world space
        // however, child actors do not operate in world space
        // they operate in a local space relative to the parent actor
        // so we have to convert
        if (!actor->isRootActor() && actor->getParentActor() != nullptr) {
            // converting world space parent matrix to local space
            Matrix4 parentModelMatrix = actor->getParentActor()->GetModelMatrix();
            Matrix4 inverseParent = MMath::inverse(parentModelMatrix);

            // inverse * T * R * S
            Matrix4 localMatrix = inverseParent * MMath::translate(worldPos) * MMath::toMatrix4(worldRotation) * MMath::scale(worldScale);

            // decomposing the local matrix to get the local translation, rotation, and scale
            float localArray[16];
            ConvertMat4toFloatArray(localMatrix, localArray);
            float localTranslation[3], localRotation[3], _localScale[3];
            ImGuizmo::DecomposeMatrixToComponents(localArray, localTranslation, localRotation, _localScale);

            // update postion, rotation, and scale according to how the gizmo was moved (local space)
            Vec3 localPos(localTranslation[0], localTranslation[1], localTranslation[2]);
            Euler localEulerRotation(localRotation[0], localRotation[1], localRotation[2]);
            Quaternion localRot = QMath::toQuaternion(localEulerRotation);
            Vec3 localScale(_localScale[0], _localScale[1], _localScale[2]);

            TC->SetTransform(localPos, localRot, localScale);
        }
        else {
            TC->SetTransform(worldPos, worldRotation, worldScale);
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

        Ref<Actor> payloadActor = Renderer::getInstance().PickActor(mx, my);

        
        const char* droppedAssetName = static_cast<const char*>(ImGui::GetDragDropPayload()->Data);

        static Ref<Component> originalComponent;
        static Ref<Actor> originalActor;
       
        //If its a MESH, make sure to colour picked based on the ORIGINAL, so restore it and redo the picking
        if (std::is_same_v<ComponentTemplate, MeshComponent> && originalActor) {
            originalActor->ReplaceComponent<ComponentTemplate>(std::dynamic_pointer_cast<ComponentTemplate>(originalComponent));
            originalActor = 0;
            originalComponent = 0;
            payloadActor = Renderer::getInstance().PickActor(mx, my);

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
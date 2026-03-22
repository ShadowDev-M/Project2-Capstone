#include "pch.h"
#include "GameWindow.h"
#include "EditorManager.h"
#include "FBOManager.h"
#include "ScreenManager.h"
#include "InputManager.h"
#include "Renderer.h"

GameWindow::GameWindow() {
    EditorManager::getInstance().RegisterWindow("Game", true);
}

void GameWindow::ShowGameWindow(bool* pOpen)
{
    if (ImGui::Begin("Game", pOpen, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_MenuBar)) {
        SceneGraph& sg = SceneGraph::getInstance();
        ScreenManager& sm = ScreenManager::getInstance();
        bool hasCamera = sg.GetMainCamera() != nullptr;
        FBOData& gameFBO = FBOManager::getInstance().getFBO(FBO::Game);

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Stats")) {
                ImGui::Text("Graphics:");
                float fps = ImGui::GetIO().Framerate;
                ImGui::Text("%.1f FPS | %.2f ms", fps, 1000.0f / fps);

                int renderW = sm.getRenderWidth();
                int renderH = sm.getRenderHeight();
                ImGui::Text("Screen: %dx%d", renderW, renderH);

                ImGui::EndMenu();
            }

            ImGui::MenuItem("Gizmos", nullptr, &showDebugGizmos);

            ImGui::EndMenuBar();
        }
        Renderer::getInstance().showGameGizmos = showDebugGizmos;

        float aspectRatio = ScreenManager::getInstance().getRenderAspectRatio();
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
        ImVec2 imagePos = ImVec2((windowSize.x - scaledTexture.x) * 0.5f, ((windowSize.y - scaledTexture.y) * 0.5f) + ImGui::GetFrameHeight());

        // Display the image with calculated dimensions
        ImGui::SetCursorPos(imagePos);
        if (hasCamera && gameFBO.isCreated) {
            ImGui::Image((void*)(intptr_t)gameFBO.texture, scaledTexture, ImVec2(0, 1), ImVec2(1, 0));
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.08f, 1.0f));
            ImGui::BeginChild("##NoCam", scaledTexture, false, ImGuiWindowFlags_NoScrollbar);
            ImVec2 sz = ImGui::CalcTextSize("No Cameras Rendering");
            ImGui::SetCursorPos(ImVec2((scaledTexture.x - sz.x) * 0.5f, (scaledTexture.y - sz.y) * 0.5f));
            ImGui::TextColored(ImVec4(0.45f, 0.45f, 0.45f, 1.0f), "No Cameras Rendering");
            ImGui::EndChild();
            ImGui::PopStyleColor();
        }
     
        InputManager::getInstance().getMouseMap()->gamePos = ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetFrameHeight());
        InputManager::getInstance().getMouseMap()->gameSize = ImGui::GetWindowSize();
        InputManager::getInstance().getMouseMap()->gameHovered = ImGui::IsWindowHovered();

        if (EditorManager::getInstance().isPlayMode()) {
            InputManager::getInstance().updateDockingFocused(ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows));
        }
    }
    ImGui::End();
}
#include "DockingWindow.h"
#include "InputManager.h"

DockingWindow::DockingWindow(SceneGraph* sceneGraph_) : sceneGraph(sceneGraph_) {}

void DockingWindow::ShowDockingWindow(bool* pOpen)
{

    
	if (ImGui::Begin("Scene Dock", pOpen, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
		

		/*ImVec2 size = ImGui::GetContentRegionAvail();
		ImGui::Image((ImTextureID)(intptr_t)SceneGraph::getInstance().dockingFBO, size, ImVec2(0, 1), ImVec2(1, 0));
       */
        
        int w, h;

        SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &w, &h);

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

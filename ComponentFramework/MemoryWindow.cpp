#include "pch.h"
#include "MemoryWindow.h"
#include "EditorManager.h"


MemoryManagerWindow::MemoryManagerWindow(SceneGraph* sceneGraph_)
{
	EditorManager::getInstance().RegisterWindow("Memory", true);

}

void MemoryManagerWindow::ShowMemoryManagerWindow(bool* pOpen)
{
    if (ImGui::Begin("Memory", pOpen, ImGuiWindowFlags_MenuBar)) {

        if (ImGui::BeginMenuBar()) {
            

            ImGui::Text( ("Memory Used: " + std::to_string(MEMORY_NUMUSEDBYTES)).c_str());



            ImGui::EndMenuBar();
        }


        ImGui::End();
    }
}

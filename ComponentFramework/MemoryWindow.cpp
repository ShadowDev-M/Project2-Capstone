#include "pch.h"
#include "MemoryWindow.h"
#include "EditorManager.h"
#include "MemorySize.h"

MemoryManagerWindow::MemoryManagerWindow(SceneGraph* sceneGraph_)
{
	EditorManager::getInstance().RegisterWindow("Memory", false);

}

void MemoryManagerWindow::ShowMemoryManagerWindow(bool* pOpen)
{
    if (ImGui::Begin("Memory", pOpen)) {

        ImGui::TextWrapped( ("Memory Used: " + std::to_string(MEMORY_NUMUSEDBYTES)).c_str());

        int staleMemory = 0;

        if (ImGui::CollapsingHeader("Blocks", ImGuiTreeNodeFlags_DefaultOpen)) {

            for (auto& pair : GetAllocMap()) {

                if (pair.second.stale) {
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
                    staleMemory++;
                }

                ImGui::TextWrapped(("|Memory Block|"));
                ImGui::Text("   Size: %d", pair.second.size);
                ImGui::Text("   File: %s", pair.second.file);
                ImGui::Text("   Line: %d", pair.second.line);

                if (pair.second.stale) {
                    ImGui::PopStyleColor();
                }

            }
        }
        else {
            for (auto& pair : GetAllocMap()) {
                if (pair.second.stale) {
                    staleMemory++;
                }
            }
        }
        ImGui::Separator();

        ImGui::TextWrapped("Stale Memory Blocks: %d", staleMemory);


    }
    ImGui::End();

}

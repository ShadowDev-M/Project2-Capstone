#include "pch.h"
#include "ProjectSettingsManager.h"

using namespace tinyxml2;

static std::string SettingsPath() { return (SearchPath::getInstance().GetEngineRoot() / ProjectSettingsManager::fileName).string(); }

bool ProjectSettingsManager::SaveDefault() const { return Save(SettingsPath()); }
bool ProjectSettingsManager::LoadDefault() { return Load(SettingsPath()); }
bool ProjectSettingsManager::Exists() const { return fs::exists(SettingsPath()); }


bool ProjectSettingsManager::Save(const std::string& filePath) const
{
    XMLDocument doc;
    XMLElement* root = doc.NewElement("ProjectSettings");
    doc.InsertFirstChild(root);
    
    // helper lamdbdas to write xml elements
    auto addStr = [&](const char* tag, const char* val) {
        XMLElement* e = doc.NewElement(tag); e->SetAttribute("value", val);
        root->InsertEndChild(e);
        };
    auto addInt = [&](const char* tag, int val) {
        XMLElement* e = doc.NewElement(tag); e->SetAttribute("value", val);
        root->InsertEndChild(e);
        };
    auto addBool = [&](const char* tag, bool val) {
        XMLElement* e = doc.NewElement(tag); e->SetAttribute("value", val ? 1 : 0);
        root->InsertEndChild(e);
        };

    addStr("windowTitle", settings.windowTitle.c_str());
    addInt("renderWidth", settings.renderWidth);
    addInt("renderHeight", settings.renderHeight);
    addInt("displayWidth", settings.displayWidth);
    addInt("displayHeight", settings.displayHeight);
    addInt("targetFPS", settings.targetFPS);
    addBool("vsync", settings.vsync);
    addStr("startupScene", settings.startupScene.c_str());

    // tags
    XMLElement* tagsEl = doc.NewElement("Tags");
    for (auto& tag : settings.tags) {
        XMLElement* te = doc.NewElement("Tag");
        te->SetAttribute("value", tag.c_str());
        tagsEl->InsertEndChild(te);
    }
    root->InsertEndChild(tagsEl);

    // scene list
    XMLElement* scenesEl = doc.NewElement("SceneList");
    for (auto& entry : settings.sceneList) {
        XMLElement* se = doc.NewElement("Scene");
        se->SetAttribute("id", entry.id);
        se->SetAttribute("name", entry.name.c_str());
        se->SetAttribute("path", entry.path.c_str());
        scenesEl->InsertEndChild(se);
    }
    root->InsertEndChild(scenesEl);

    return doc.SaveFile(filePath.c_str()) == XML_SUCCESS;
}

bool ProjectSettingsManager::Load(const std::string& filePath)
{
    XMLDocument doc;
    if (doc.LoadFile(filePath.c_str()) != XML_SUCCESS) return false;
    XMLElement* root = doc.RootElement();
    if (!root) return false;

    // helper lamdbdas to read xml elements
    auto readStr = [&](const char* tag, std::string& out) {
        auto* e = root->FirstChildElement(tag);
        if (e) { const char* v = e->Attribute("value"); if (v) out = v; }
        };
    auto readInt = [&](const char* tag, int& out) {
        auto* e = root->FirstChildElement(tag);
        if (e) out = e->IntAttribute("value", out);
        };
    auto readBool = [&](const char* tag, bool& out) {
        auto* e = root->FirstChildElement(tag);
        if (e) out = (e->IntAttribute("value", out ? 1 : 0) != 0);
        };

    readStr("windowTitle", settings.windowTitle);
    readInt("renderWidth", settings.renderWidth);
    readInt("renderHeight", settings.renderHeight);
    readInt("displayWidth", settings.displayWidth);
    readInt("displayHeight", settings.displayHeight);
    readInt("targetFPS", settings.targetFPS);
    readBool("vsync", settings.vsync);
    readStr("startupScene", settings.startupScene);

    // tags
    settings.tags.clear();
    auto* tagsEl = root->FirstChildElement("Tags");
    if (tagsEl) {
        for (auto* te = tagsEl->FirstChildElement("Tag"); te;
            te = te->NextSiblingElement("Tag")) {
            const char* v = te->Attribute("value");
            if (v) settings.tags.push_back(v);
        }
    }
    if (settings.tags.empty()) settings.tags = { "Untagged", "MainCamera", "Player", "Ground" };

    // scene list
    settings.sceneList.clear();
    auto* scenesEl = root->FirstChildElement("SceneList");
    if (scenesEl) {
        for (auto* se = scenesEl->FirstChildElement("Scene"); se; se = se->NextSiblingElement("Scene")) {
            SceneListData data;
            data.id = se->IntAttribute("id", 0);
            const char* n = se->Attribute("name"); if (n) data.name = n;
            const char* p = se->Attribute("path"); if (p) data.path = p;
            settings.sceneList.push_back(data);
        }
    }

    settings.RebuildIds();
    return true;
}

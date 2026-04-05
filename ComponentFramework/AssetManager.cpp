#include "pch.h"
#include "AssetManager.h"
#include "ScriptAbstract.h"
#include "AnimationSystem.h"
#include "Renderer.h"
#include "SceneGraph.h"
#include "ShadowSettings.h"
#include "TilingSettings.h"

void AssetManager::LoadEngineAssets()
{
    SearchPath& sp = SearchPath::getInstance();
    const fs::path& engineRoot = sp.GetEngineRoot();
    if (engineRoot.empty() || !fs::exists(engineRoot)) return;

    // helper lambdas for interal loading of component types
    auto loadEngineMesh = [&](const fs::path& path) {
        std::string stem = path.stem().string();
        AssetKey key{ "__engine__" + stem, "MeshComponent" };
        if (engineAssetMap.count(key)) return;
        Ref<MeshComponent> mesh = std::make_shared<MeshComponent>(nullptr, path.string().c_str());
        AnimationSystem::getInstance().PushMeshToWorker(mesh);
        engineAssetMap[key] = mesh;
        };
    auto loadEngineTexture = [&](const fs::path& path) {
        std::string stem = path.stem().string();
        AssetKey key{ "__engine__" + stem, "MaterialComponent" };
        if (engineAssetMap.count(key)) return;
        Ref<MaterialComponent> mat = std::make_shared<MaterialComponent>(nullptr, path.string().c_str());
        mat->OnCreate();
        engineAssetMap[key] = mat;
        };

    for (auto& a : sp.FindByExtension(sp.GetEngineRoot(), "Meshes", { ".obj", ".fbx" })) loadEngineMesh(a);
    for (auto& a : sp.FindByExtension(sp.GetEngineRoot(), "Textures", { ".png", ".jpg", ".jpeg" })) loadEngineTexture(a);
    for (auto& a : sp.FindByExtension(sp.GetEngineRoot(), "Icons", { ".png", ".jpg", ".jpeg" })) loadEngineTexture(a);
}

void AssetManager::Refresh() {
    // snapshot of current maps, not a reference a full copy
    auto oldMap = assetMap;
    auto oldPaths = assetPaths;

    SearchPath& sp = SearchPath::getInstance();
    
    // for assets with component types
    struct FileLoad { fs::path path; std::string folder; };
    std::vector<FileLoad> allFiles;

    // basically this means that for a specific file to be loaded as a specific component, it must be in a specific folder
    // this was the easiest way I could think of so that different file types get loaded in properly as their respective components
    // because if I just search through one big open directory for all file types, things like .fbx meshes but turn into .fbx animations
    // it is recursive though, so we could do like Scripts/Player, or Animations/Player, it just needs that subfolder
    for (auto& a : sp.FindByExtension(sp.GetRoot(), "Meshes", { ".obj", ".fbx" })) allFiles.push_back({ a, "Meshes" });
    for (auto& a : sp.FindByExtension(sp.GetRoot(), "Textures", { ".png", ".jpg", ".jpeg" })) allFiles.push_back({ a, "Textures" });
    for (auto& a : sp.FindByExtension(sp.GetRoot(), "Materials", { ".mat" })) allFiles.push_back({ a, "Materials" });
    for (auto& a : sp.FindByExtension(sp.GetRoot(), "Shaders", { ".shader" })) allFiles.push_back({ a, "Shaders" });
    for (auto& a : sp.FindByExtension(sp.GetRoot(), "Scripts", { ".lua" })) allFiles.push_back({ a, "Scripts" });
    for (auto& a : sp.FindByExtension(sp.GetRoot(), "Animations", { ".gltf", ".fbx" })) allFiles.push_back({ a, "Animations" });

    std::unordered_set<AssetKey, AssetKeyHasher> currentKeys;

    // .shader manifests are the actual shader components, these are purely just to create them
    // could always do a rename thing though where its no longer .glsl, and split into .frag, .vert, etc, and it detects via same names
    // would still face the same issues though when creating shaders, how can you tell apart a shader that has different naming schemes i.e outline.vert and phong.frag
    rawGLSLPaths.clear();
    for (auto& a : sp.FindByExtension(sp.GetRoot(), "Shaders", { ".glsl" })) rawGLSLPaths.push_back(a); 
    
    scenePaths.clear();
    for (auto& a : sp.FindByExtension(sp.GetRoot(), "Scenes", { ".scene" })) LoadScene(a);
    
    prefabPaths.clear();
    for (auto& a : sp.FindByExtension(sp.GetRoot(), "Prefabs", { ".prefab" })) LoadPrefab(a);

    // now checks each indivual file and folder, and only reloads if it detects the file has been modifed or is a new file
    for (auto& [path, folder] : allFiles) {
        std::string ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower); // convert to lowercase for extension check
        std::string stem = path.stem().string();

        // finding the component type by file extension
        std::string type;
        if (ext == ".obj" || (ext == ".fbx" && folder == "Meshes")) type = "MeshComponent";
        else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".mat") type = "MaterialComponent";
        else if (ext == ".shader") type = "ShaderComponent";
        else if (ext == ".lua") type = "ScriptAbstract";
        else if (ext == ".gltf" || (ext == ".fbx" && folder == "Animations")) type = "Animation";

        if (type.empty()) continue;

        AssetKey key{ stem, type };
        currentKeys.insert(key);

        if (!HasFileChanged(path)) {
            if (assetMap.count(key)) continue;
        }

        assetMap.erase(key);
        assetPaths.erase(key);

        Ref<Component> oldComp; 
        auto oldIt = oldMap.find(key);
        if (oldIt != oldMap.end()) oldComp = oldIt->second;

        if (type == "MeshComponent") LoadMesh(path);
        else if (type == "MaterialComponent" && ext == ".mat") LoadMatManifest(path);
        else if (type == "MaterialComponent") LoadTexture(path);
        else if (type == "ShaderComponent") LoadShaderManifest(path);
        else if (type == "ScriptAbstract") LoadScript(path);
        else if (type == "Animation") LoadAnimation(path);

        RecordFileTime(path);

        if (oldComp) {
            auto newIt = assetMap.find(key);
            if (newIt != assetMap.end() && newIt->second.get() != oldComp.get()) {
                ReplaceComponent(oldComp, newIt->second, type);
            }
        }
    }
    
    for (auto& [key, oldComp] : oldMap) {
        if (!currentKeys.count(key)) {
            NotifyActors(key.name, key.componentType, oldComp);
        }
    }
}

void AssetManager::RefreshSingle(const fs::path& absolutePath) {
    std::string ext = absolutePath.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower); // convert to lowercase for extension check
    std::string stem = absolutePath.stem().string();

    // finding the component type by file extension
    std::string type;
    if (ext == ".obj" || ext == ".fbx") type = "MeshComponent";
    else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".mat") type = "MaterialComponent";
    else if (ext == ".shader") type = "ShaderComponent";
    else if (ext == ".lua") type = "ScriptAbstract";
    else if (ext == ".gltf") type = "Animation";

    // saves the oldcomponent as a reference
    Ref<Component> oldComponent;
    if (!type.empty()) {
        AssetKey key{ stem, type };
        auto it = assetMap.find(key);
        if (it != assetMap.end()) {
            oldComponent = it->second;
            assetMap.erase(it);
            assetPaths.erase(key);
        }
    }
    else if (ext == ".glsl") rawGLSLPaths.erase(std::remove(rawGLSLPaths.begin(), rawGLSLPaths.end(), absolutePath), rawGLSLPaths.end());
    else if (ext == ".scene") scenePaths.erase(std::remove(scenePaths.begin(), scenePaths.end(), absolutePath), scenePaths.end());
    else if (ext == ".prefab") prefabPaths.erase(std::remove(prefabPaths.begin(), prefabPaths.end(), absolutePath), prefabPaths.end());

    // if the file still exists/wasn't deleted
    if (!fs::exists(absolutePath)) return;
    if (ext == ".obj" || ext == ".fbx") {
        std::string folder = absolutePath.parent_path().filename().string();
        if (folder == "Animations") LoadAnimation(absolutePath);
        else LoadMesh(absolutePath);
    }
    else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") LoadTexture(absolutePath);
    else if (ext == ".mat") LoadMatManifest(absolutePath);
    else if (ext == ".shader") LoadShaderManifest(absolutePath);
    else if (ext == ".lua") LoadScript(absolutePath);
    else if (ext == ".gltf") LoadAnimation(absolutePath);
    else if (ext == ".glsl") rawGLSLPaths.push_back(absolutePath);
    else if (ext == ".scene") LoadScene(absolutePath);
    else if (ext == ".prefab") LoadPrefab(absolutePath);

    // hotswap ptr
    if (oldComponent && !type.empty()) {
        AssetKey newKey{ stem,type };
        auto newIt = assetMap.find(newKey);
        if (newIt != assetMap.end() && newIt->second.get() != oldComponent.get()) {
            ReplaceComponent(oldComponent, newIt->second, type);
        }
    }
}

bool AssetManager::RenameAsset(const std::string& oldName, const std::string& newName, const std::string& componentType) {
    AssetKey oldKey{ oldName, componentType };
    auto pathIt = assetPaths.find(oldKey);
    if (pathIt == assetPaths.end()) return false;

    fs::path oldPath = pathIt->second;
    fs::path newPath = oldPath.parent_path() / (newName + oldPath.extension().string());

    std::error_code ec;
    fs::rename(oldPath, newPath, ec);
    if (ec) return false;

    auto assetIt = assetMap.find(oldKey);
    if (assetIt != assetMap.end()) {
        AssetKey newKey{ newName, componentType };
        assetMap[newKey] = assetIt->second;
        assetPaths[newKey] = newPath;
        assetMap.erase(oldKey);
        assetPaths.erase(oldKey);
    }

    return true;
}

bool AssetManager::DuplicateAsset(const std::string& name, const std::string& componentType)
{
    AssetKey key{ name, componentType };
    auto pathIt = assetPaths.find(key);
    if (pathIt == assetPaths.end()) return false;

    fs::path src = pathIt->second;
    std::string newStem = GenerateUniqueFileName(src.parent_path(), name, src.extension().string());
    fs::path dst = src.parent_path() / (newStem + src.extension().string());

    std::error_code ec;
    fs::copy_file(src, dst, ec);
    if (ec) return false;

    RefreshSingle(dst);

    return true;
}

bool AssetManager::DeleteAsset(const std::string& name, const std::string& componentType)
{
    AssetKey key{ name, componentType };
    auto pathIt = assetPaths.find(key);
    if (pathIt == assetPaths.end()) return false;

    // save the deleted component and notify actors
    Ref<Component> delComp = assetMap.count(key) ? assetMap.at(key) : nullptr;
    if (delComp) NotifyActors(name, componentType, delComp);

    std::error_code ec;
    fs::remove(pathIt->second, ec);
    if (ec) return false;

    assetMap.erase(key);
    assetPaths.erase(key);

    return true;
}

bool AssetManager::MoveAsset(const std::string& name, const std::string& componentType, const fs::path& destination)
{
    AssetKey key{ name, componentType };
    auto pathIt = assetPaths.find(key);
    if (pathIt == assetPaths.end()) return false;
    
    fs::path src = pathIt->second;
    fs::path dst = destination / src.filename();
    if (src == dst) return true;

    std::error_code ec;
    fs::rename(src, dst, ec);
    if (ec) return false;

    assetPaths[key] = dst;
    return true;
}

std::string AssetManager::GenerateUniqueFileName(const fs::path& dir, const std::string& stem, const std::string& ext)
{
    // based on generateuniquename for actors but this time for files
    std::string baseStem = stem;
    size_t underscore = stem.find_last_of('_');
    if (underscore != std::string::npos) {
        size_t d = stem.find_last_of('D');
        if (d == stem.length() - 1 && d > underscore)
            baseStem = stem.substr(0, underscore);
    }

    if (!fs::exists(dir / (baseStem + ext))) return baseStem;

    int counter = 1;
    std::string uniqueStem;
    do {
        uniqueStem = baseStem + "_" + std::to_string(counter++) + "D";
    } while (fs::exists(dir / (uniqueStem + ext)));

    return uniqueStem;
}

void AssetManager::NotifyActors(const std::string& name, const std::string& componentType, const Ref<Component>& deletedComponent) {
    if (!deletedComponent) return;

    Ref<MaterialComponent> fallbackMat = Renderer::getInstance().GetFallBackMaterial();
    Ref<ShaderComponent> fallbackShader = Renderer::getInstance().GetFallBackShader();

    // goes through all actors in scenegraphs and either replaces or deltes the component
    // its either create a fallback component for each component type and replace, or just delete the component
    // I think its fine to just delete the component if it was using a deleted asset, but could create something later that resets them
    for (auto& [id, actor] : SceneGraph::getInstance().getAllActors()) {
        if (componentType == "MaterialComponent") {
            auto c = actor->GetComponent<MaterialComponent>();
            if (c && c.get() == deletedComponent.get()) {
                if (fallbackMat) actor->ReplaceComponent<MaterialComponent>(fallbackMat);
                else {
                    actor->RemoveComponent<MaterialComponent>();
                    if (actor->GetComponent<TilingSettings>()) actor->DeleteComponent<TilingSettings>();
                }
            }
        }
        else if (componentType == "ShaderComponent") {
            auto c = actor->GetComponent<ShaderComponent>();
            if (c && c.get() == deletedComponent.get()) {
                if (fallbackShader) actor->ReplaceComponent<ShaderComponent>(fallbackShader);
                else actor->RemoveComponent<ShaderComponent>();
            }
        }
        else if (componentType == "MeshComponent") {
            auto c = actor->GetComponent<MeshComponent>();
            if (c && c.get() == deletedComponent.get()) {
                actor->RemoveComponent<MeshComponent>();
                if (actor->GetComponent<ShadowSettings>()) actor->DeleteComponent<ShadowSettings>();
                if (actor->GetComponent<AnimatorComponent>()) actor->DeleteComponent<AnimatorComponent>();
            }
        }
        else if (componentType == "ScriptAbstract") {
            auto scripts = actor->GetAllComponent<ScriptComponent>();
            for (int i = (int)scripts.size() - 1; i >= 0; i--) {
                if (scripts[i]->getBaseAsset().get() == deletedComponent.get()) {
                    ScriptService::stopActorScripts(actor);
                    actor->DeleteComponent<ScriptComponent>(i);
                }
            }
        }
        else if (componentType == "Animation") {
            auto ac = actor->GetComponent<AnimatorComponent>();
            if (ac) {
                AnimationClip clip = ac->getAnimationClip();
                if (clip.getAnim() && clip.getAnim().get() == deletedComponent.get()) {
                    ac->StopClip();
                    ac->setAnimationClip(AnimationClip{});
                }
            }
        }
    }
}

void AssetManager::ReplaceComponent(const Ref<Component>& oldComponent, const Ref<Component>& newComponent, const std::string& componentType) {
    if (!oldComponent || !newComponent) return;

    // basically just a dispatcher that repalces old component with new
    for (auto& [id, actor] : SceneGraph::getInstance().getAllActors()) {
        if (componentType == "MaterialComponent") {
            auto c = actor->GetComponent<MaterialComponent>();
            if (c && c.get() == oldComponent.get()) actor->ReplaceComponent<MaterialComponent>(std::dynamic_pointer_cast<MaterialComponent>(newComponent));
        }
        else if (componentType == "ShaderComponent") {
            auto c = actor->GetComponent<ShaderComponent>();
            if (c && c.get() == oldComponent.get()) actor->ReplaceComponent<ShaderComponent>(std::dynamic_pointer_cast<ShaderComponent>(newComponent));
        }
        else if (componentType == "MeshComponent") {
            auto c = actor->GetComponent<MeshComponent>();
            if (c && c.get() == oldComponent.get()) actor->ReplaceComponent<MeshComponent>(std::dynamic_pointer_cast<MeshComponent>(newComponent));
        }
    }
}

fs::path AssetManager::GetAssetPath(const std::string& name, const std::string& componentType) const
{
    auto it = assetPaths.find({ name, componentType });
    return (it != assetPaths.end()) ? it->second : fs::path{};
}

std::string AssetManager::GetAssetName(const Ref<Component>& component) const {
    for (auto& [key, val] : assetMap) if (val == component) return key.name;
    return {};
}

std::vector<Ref<Component>> AssetManager::GetAllAssets() const
{
    std::vector<Ref<Component>> all;
    all.reserve(assetMap.size());
    for (auto& [key, val] : assetMap) all.push_back(val);
    return all;
}

std::vector<std::pair<std::string, std::string>> AssetManager::GetAllAssetKeyPair() const
{
    std::vector<std::pair<std::string, std::string>> pairs;
    for (auto& [key, val] : assetMap) pairs.emplace_back(key.name, key.componentType);
    return pairs;
}

bool AssetManager::HasFileChanged(const fs::path& path) const
{
    std::error_code ec;
    auto lastWrite = fs::last_write_time(path, ec);
    if (ec) return true;

    auto it = fileTimes.find(path.string());
    if (it == fileTimes.end()) return true;
    return it->second != lastWrite;
}

void AssetManager::RecordFileTime(const fs::path& path)
{
    std::error_code ec;
    auto t = fs::last_write_time(path, ec);
    if (!ec) fileTimes[path.string()] = t;
}

void AssetManager::LoadMesh(const fs::path& path) {
    std::string stem = path.stem().string();
    AssetKey key{ stem, "MeshComponent" };
    if (assetMap.count(key)) return;

    auto mesh = std::make_shared<MeshComponent>(nullptr, path.string().c_str(), true);
    if (mesh->OnCreate()) {
        assetMap[key] = mesh;
        assetPaths[key] = path;
        AnimationSystem::getInstance().PushMeshToWorker(mesh);
    }
}

void AssetManager::LoadTexture(const fs::path& path) {
    std::string stem = path.stem().string();
    AssetKey key{ stem, "MaterialComponent" };
    if (assetMap.count(key)) return;

    auto mat = std::make_shared<MaterialComponent>(nullptr, path.string().c_str(), "", "");
    if (mat->OnCreate()) {
        assetMap[key] = mat;
        assetPaths[key] = path;
    }
}

void AssetManager::LoadMatManifest(const fs::path& path) {
    XMLDocument doc;
    if (doc.LoadFile(path.string().c_str()) != XML_SUCCESS) return;

    auto* root = doc.FirstChildElement("Material");
    if (!root) return;

    // lambda helper to get value from an absolute path
    auto getPath = [&](const char* tag) -> std::string {
        auto* el = root->FirstChildElement(tag);
        if (!el || !el->GetText()) return "";
        fs::path abs = SearchPath::getInstance().Resolve(el->GetText());
        return abs.empty() ? "" : abs.string();
        };

    std::string diff = getPath("Diffuse");
    std::string spec = getPath("Specular");
    std::string norm = getPath("Normal");

    if (diff.empty()) return;

    auto mat = std::make_shared<MaterialComponent>(nullptr, diff.c_str(), spec.c_str(), norm.c_str());

    if (mat->OnCreate()) {
        std::string name = path.stem().string();
        AssetKey key{ name, "MaterialComponent" };
        assetMap[key] = mat;
        assetPaths[key] = path;
    }
}

void AssetManager::LoadShaderManifest(const fs::path& path)
{
    XMLDocument doc;
    if (doc.LoadFile(path.string().c_str()) != XML_SUCCESS) return;

    auto* root = doc.FirstChildElement("Shader");
    if (!root) return;

    // lambda helper to get value from an absolute path
    auto getPath = [&](const char* tag) -> std::string {
        auto* el = root->FirstChildElement(tag);
        if (!el || !el->GetText()) return "";
        fs::path abs = SearchPath::getInstance().Resolve(el->GetText());
        return abs.empty() ? "" : abs.string();
        };

    std::string vert = getPath("Vertex");
    std::string frag = getPath("Fragment");
    std::string tessCtrl = getPath("TessControl");
    std::string tessEval = getPath("TessEval");
    std::string geom = getPath("Geometry");

    if (vert.empty() || frag.empty()) return;

    auto shader = std::make_shared<ShaderComponent>(nullptr,
        vert.c_str(), frag.c_str(),
        tessCtrl.empty() ? nullptr : tessCtrl.c_str(),
        tessEval.empty() ? nullptr : tessEval.c_str(),
        geom.empty() ? nullptr : geom.c_str());

    if (shader->OnCreate()) {
        std::string name = path.stem().string();
        AssetKey key{ name, "ShaderComponent" };
        assetMap[key] = shader;
        assetPaths[key] = path;
    }
}

void AssetManager::LoadScript(const fs::path& path) {
    std::string name = path.stem().string();
    AssetKey key{ name, "ScriptAbstract" };
    if (assetMap.count(key)) return;

    auto script = std::make_shared<ScriptAbstract>(nullptr, path.filename().string().c_str());
    script->OnCreate();
    assetMap[key] = script;
    assetPaths[key] = path;
}

void AssetManager::LoadAnimation(const fs::path& path) {
    std::string name = path.stem().string();
    AssetKey key{ name, "Animation" };
    if (assetMap.count(key)) return;

    auto anim = std::make_shared<Animation>(nullptr, path.string().c_str());
    anim->SendAssetname(name);
    if (anim->OnCreate()) {
        assetMap[key] = anim;
        assetPaths[key] = path;
        AnimationSystem::getInstance().PushAnimationToWorker(anim);
    }
}

void AssetManager::LoadScene(const fs::path& path) {
    auto it = std::find(scenePaths.begin(), scenePaths.end(), path);
    if (it == scenePaths.end()) scenePaths.push_back(path);
}

void AssetManager::LoadPrefab(const fs::path& path) {
    auto it = std::find(prefabPaths.begin(), prefabPaths.end(), path);
    if (it == prefabPaths.end()) prefabPaths.push_back(path);
}
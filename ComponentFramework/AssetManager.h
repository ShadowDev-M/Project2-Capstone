#pragma once
#include "ScriptAbstract.h"
#include "MaterialComponent.h"
#include "MeshComponent.h"
#include "ShaderComponent.h"
#include "TransformComponent.h"
#include "AnimatorComponent.h"
#include "MemoryRecorder.h"

using namespace tinyxml2;

struct AssetKey {
	std::string name;
	std::string componentType;

	bool operator == (const AssetKey& other) const {
		return name == other.name && componentType == other.componentType;
	}
};

struct AssetKeyHasher {
	std::size_t operator()(const AssetKey& key) const {
		std::hash<std::string> hasherStr;
		return hasherStr(key.name) ^
			(hasherStr(key.componentType) << 1);
	}
};

class AssetManager
{
	AssetManager() = default;
	AssetManager(const AssetManager&) = delete;
	AssetManager(AssetManager&&) = delete;
	AssetManager& operator = (const AssetManager&) = delete;
	AssetManager& operator = (AssetManager&&) = delete;
	
	// unordered map that stores all the assets names and component types, 
	std::unordered_map<AssetKey, Ref<Component>, AssetKeyHasher> assetMap;
	// unordered map that stores the assets filepaths
	std::unordered_map<AssetKey, fs::path, AssetKeyHasher> assetPaths;

	// for engine assets
	std::unordered_map<AssetKey, Ref<Component>, AssetKeyHasher> engineAssetMap;

	// timestamp based refresh, only will refresh new or modified files
	std::unordered_map<std::string, fs::file_time_type> fileTimes;
	bool HasFileChanged(const fs::path& path) const;
	void RecordFileTime(const fs::path& path);
	
	// giving each asset type its own loading function, its just easier to manage all the different assets
	void LoadMesh(const fs::path& path);
	void LoadTexture(const fs::path& path);
	void LoadMatManifest(const fs::path& path);
	void LoadShaderManifest(const fs::path& path);
	void LoadScript(const fs::path& path);
	void LoadAnimation(const fs::path& path);
	void LoadScene(const fs::path& path);
	void LoadPrefab(const fs::path& path);

	// if an actor is using an asset thats been deleted
	// need to notify that actor and get rid of that asset
	void NotifyActors(const std::string& name, const std::string& componentType, const Ref<Component>& deletedComponent);
	
	// used for RefreshSingle so that when a manifest is edited and the ptr is updated,
	// the actors recieve the new ptr
	void ReplaceComponent(const Ref<Component>& oldComponent, const Ref<Component>& newComponent, const std::string& componentType);

	// non-component files
	std::vector<fs::path> rawGLSLPaths;
	std::vector<fs::path> scenePaths;
	std::vector<fs::path> prefabPaths;

public:
	// Meyers Singleton (from JPs class)
	static AssetManager& getInstance() {
		static AssetManager instance;
		return instance;
	}

	~AssetManager() { RemoveAllAssets(); }

	bool Initialize() { Refresh(); return true; }
	void RemoveAllAssets() { assetMap.clear(); assetPaths.clear(); }

	// engine initialize
	void LoadEngineAssets();

	// debated on having a filewatcher, or just doing manual refreshes wherever needed,
	// settled on manual refreshes, in the editor whenever something changes it'll refresh automatically, 
	// it just means we have to manually do a refresh whenever we add files via the file explorer/outside the engine
	void Refresh();

	// if a single file needs changes its cheaper to just refresh that single file then doing the entire map
	void RefreshSingle(const fs::path& absolutePath);

	template<typename AssetTemplate, typename ... Args>
	bool AddAsset(const std::string& name_, Args&& ... args_) { 
		std::string componentType = static_cast<std::string>(typeid(AssetTemplate).name()).substr(6);
		AssetKey key = { name_, componentType };

		// check to see if theres already an asset in the assetmanager with the same name
		if (assetMap.count(key)) {
			Debug::Warning("Asset: " + name_ + " already exists.", __FILE__, __LINE__);
			return false;
		}

		// add asset to assetmanager
		RECORD Ref<AssetTemplate> asset = std::make_shared<AssetTemplate>(std::forward<Args>(args_)...);
		assetMap[key] = asset;
		return true;
	}

	template<typename AssetTemplate>
	Ref<AssetTemplate> GetAsset(const std::string& assetName_) {
		std::string componentType = static_cast<std::string>(typeid(AssetTemplate).name()).substr(6);
		AssetKey key = { assetName_, componentType };

		// if asset is found return it
		auto it = assetMap.find(key);
		if (it != assetMap.end()) {
			return std::dynamic_pointer_cast<AssetTemplate>(it->second);
		}

		// if asset isnt found, throw error
		else {
			Debug::Error("Can't find requested asset: ", __FILE__, __LINE__);
			return nullptr;
		}
	}

	template<typename AssetTemplate>
	Ref<AssetTemplate> GetEngineAsset(const std::string& assetName_) {
		std::string componentType = static_cast<std::string>(typeid(AssetTemplate).name()).substr(6);
		AssetKey key = { "__engine__" + assetName_, componentType};

		// if asset is found return it
		auto it = engineAssetMap.find(key);
		if (it != engineAssetMap.end()) {
			return std::dynamic_pointer_cast<AssetTemplate>(it->second);
		}

		// if asset isnt found, throw error
		else {
			Debug::Error("Can't find requested asset: ", __FILE__, __LINE__);
			return nullptr;
		}
	}

	template<typename T>
	bool HasAsset(const std::string& name) const {
		std::string type = std::string(typeid(T).name()).substr(6);
		return assetMap.count(AssetKey{ name, type }) > 0;
	}

	template<typename AssetTemplate>
	std::vector<std::pair<std::string, Ref<AssetTemplate>>> GetAllOfType() const {
		std::string type = std::string(typeid(AssetTemplate).name()).substr(6);
		std::vector<std::pair<std::string, Ref<AssetTemplate>>> results;
		for (auto& [key, val] : assetMap) {
			if (key.componentType == type) {
				auto typed = std::dynamic_pointer_cast<AssetTemplate>(val);
				if (typed) results.emplace_back(key.name, typed);
			}
		}

		return results;
	}

	// ProjectWindow functions
	bool RenameAsset(const std::string& oldName, const std::string& newName, const std::string& componentType);
	bool DuplicateAsset(const std::string& name, const std::string& componentType);
	bool DeleteAsset(const std::string& name, const std::string& componentType);
	bool MoveAsset(const std::string& name, const std::string& componentType, const fs::path& destination);
	std::string GenerateUniqueFileName(const fs::path& dir, const std::string& stem, const std::string& ext);

	fs::path GetAssetPath(const std::string& name, const std::string& componentType) const;
	std::string GetAssetName(const Ref<Component>& component) const;
	std::vector<Ref<Component>> GetAllAssets() const;
	std::vector<std::pair<std::string, std::string>> GetAllAssetKeyPair() const;

	// there are certain file types that are no component related, and will not be loaded in
	// so these have to be created separately
	const std::vector<fs::path>& GetRawGLSLPaths() const { return rawGLSLPaths; }
	const std::vector<fs::path>& GetScenePaths() const { return scenePaths; }
	const std::vector<fs::path>& GetPrefabPaths() const { return prefabPaths; }
};

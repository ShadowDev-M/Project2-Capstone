#pragma once

#include "ScriptAbstract.h"
#include "MaterialComponent.h"
#include "MeshComponent.h"
#include "ShaderComponent.h"
#include "TransformComponent.h"
#include "AnimatorComponent.h"
using namespace tinyxml2;

struct AssetKey
{
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
private:
	AssetManager() {}
	
	// delete copy and move constructers
	AssetManager(const AssetManager&) = delete;
	AssetManager(AssetManager&&) = delete;
	AssetManager& operator = (const AssetManager&) = delete;
	AssetManager& operator = (AssetManager&&) = delete;
	
	// unordered map that stores all the assets names, component types, and filepaths
	std::unordered_map<AssetKey, Ref<Component>, AssetKeyHasher> assetManager;

	// TODO: XML is only temporary, its set up just to have a persistent database that can add and remove assets from the asset manager for basic engine functionality, 
	// but in the long run it would be better to have a fully functional file browser similar to Unity
	
	// asset database XML file name
	std::string assetDatabasePath = "AssetDatabase";

	// helper method that reads the asset database XML file for a specific type of asset (used in LoadAssetDatabaseXML to actually load the assets)
	template<typename AssetTemplate>
	bool LoadAssetTypeXML(XMLElement* assetElement_, const std::string& assetName_);

public:

	// Meyers Singleton (from JPs class)
	static AssetManager& getInstance() {
		static AssetManager instance;
		return instance;
	}

	~AssetManager() { RemoveAllAssets(); }

	template<typename AssetTemplate, typename ... Args>
	bool AddAsset(const std::string& name_, Args&& ... args_) { 
		std::string componentType = static_cast<std::string>(typeid(AssetTemplate).name()).substr(6);
		AssetKey key = { name_, componentType };


		Ref<AssetTemplate> asset = std::make_shared<AssetTemplate>(std::forward<Args>(args_)...);

	

		// check to see if theres already an asset in the assetmanager with the same name
		auto it = assetManager.find(key);
		if (it != assetManager.end()) {
			Debug::Warning("Asset: " + name_ + " already exists.", __FILE__, __LINE__);
			return false;
		}

		// add asset to assetmanager
		assetManager[key] = asset;
		return true;
	}

	template<typename AssetTemplate>
	Ref<AssetTemplate> GetAsset(const std::string& assetName_) {
		std::string componentType = static_cast<std::string>(typeid(AssetTemplate).name()).substr(6);
		AssetKey key = { assetName_, componentType };
		auto it = assetManager.find(key);

		// if asset is found return it
		if (it != assetManager.end()) {
			return std::dynamic_pointer_cast<AssetTemplate>(it->second);
		}

		// if asset isnt found, throw error
		else {
			Debug::Error("Can't find requested asset: ", __FILE__, __LINE__);
			return Ref<AssetTemplate>(nullptr);
		}
	}

	std::vector<Ref<Component>> GetAllAssets() const {
		std::vector<Ref<Component>> allAssets;

		for (auto& pair : assetManager) {
			allAssets.push_back(pair.second);
		}
		return allAssets;
	}
	
	std::vector<std::string> GetAllAssetNames() const {
		std::vector<std::string> allAssetNames;

		for (auto& pair : assetManager) {
			allAssetNames.push_back(pair.first.name);
		}
		return allAssetNames;
	}

	std::vector<std::pair<std::string, std::string>> GetAllAssetKeyPair() const {
		std::vector<std::pair<std::string, std::string>> allAssetNames;

		for (auto& pair : assetManager) {
			allAssetNames.push_back(std::make_pair(pair.first.name, pair.first.componentType));
		}
		return allAssetNames;
	}

	/// returns name of component
	std::string getAssetName(const Ref<Component>& component_) {
    for (auto& [key, value] : assetManager) {
        if (value == component_) {
            return key.name;
        }
    }

    // Not found, handle better to not explode via nothing being returned :/
    return {};
}

	// lists assets
	void ListAllAssets() const {
		std::cout << "Asset Manager Currently Holds These Assets: " << std::endl;
		for (auto it = assetManager.begin(); it != assetManager.end(); it++) {
			std::cout << it->first.name << std::endl;
		}
	}

	// removes assets
	void RemoveAllAssets() {
		std::cout << "Deleting All Assets In Asset Manager" << std::endl;
		assetManager.clear();
	}

	bool OnCreate();

	//
	std::string GetAssetDatabasePath() { return assetDatabasePath; }

	// save all assets to the asset database
	bool SaveAssetDatabaseXML() const;

	// removes an asset by name
	template<typename AssetTemplate>
	bool RemoveAsset(const std::string& assetName_);

	// loads the asset database xml file
	bool LoadAssetDatabaseXML();

	// reloads all the assets
	bool ReloadAssetsXML();

	// adds, gets, and calls the oncreate of a specific asset used when creating an asset in scene
	template<typename AssetTemplate, typename ... Args>
	bool LoadAsset(const std::string& assetName_, Args&& ... args_) {
		// get the typeID for a compontent and return its name
		std::string componentType = static_cast<std::string>(typeid(AssetTemplate).name()).substr(6);

		// add the specific asset to the assetmanager + error handling (AddAsset already handles if there is a duplicate asset trying to be added)
		bool result = AddAsset<AssetTemplate>(assetName_, std::forward<Args>(args_)...);
		if (!result) {
			Debug::Error("Failed to add " + componentType + " to the Asset Manager: " + assetName_, __FILE__, __LINE__);
			return false;
		}
		auto asset = GetAsset<AssetTemplate>(assetName_);
		if (!asset) {
			Debug::Error("Failed to retrieve " + componentType + "after adding: " + assetName_, __FILE__, __LINE__);
			RemoveAsset<AssetTemplate>(assetName_);
			return false;
		}
		if (!asset->OnCreate()) {
			RemoveAsset<AssetTemplate>(assetName_);
			Debug::Error("Failed to initialize " + componentType + ". This asset's OnCreate failed: " + assetName_, __FILE__, __LINE__);
			return false;
		}

		// TODO change this because it saves the entire database
		SaveAssetDatabaseXML();

		// if the asset manages to pass everything, then display an info debug message
		Debug::Info("Succesfully loaded " + componentType + ": " + assetName_, __FILE__, __LINE__);
		return true;
	}
};

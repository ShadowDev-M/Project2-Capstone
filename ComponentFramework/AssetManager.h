#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include "Debug.h"

#include "MaterialComponent.h"
#include "MeshComponent.h"
#include "ShaderComponent.h"
#include "TransformComponent.h"


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
	
	// const char* creates an address for the name and shares it between same names
	// std::string compares strings but costs more memory
	std::unordered_map<AssetKey, Ref<Component>, AssetKeyHasher> assetManager;

public:

	// Meyers Singleton (from JPs class)
	static AssetManager& getInstance() {
		static AssetManager instance;
		return instance;
	}

	~AssetManager() { RemoveAllAssets(); }

	template<typename AssetTemplate, typename ... Args>
	bool AddAsset(std::string name_, Args&& ... args_) { 
		std::string componentType = static_cast<std::string>(typeid(AssetTemplate).name()).substr(6);
		AssetKey key = { name_, componentType };


		Ref<AssetTemplate> asset = std::make_shared<AssetTemplate>(std::forward<Args>(args_)...);

	

		// check to see if theres already an asset in the assetmanager with the same name
		auto it = assetManager.find(key);
		if (it != assetManager.end()) {
			Debug::Warning("Asset: " + name_ + "| already exists.", __FILE__, __LINE__);
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
			Debug::Error("Can't fint requested asset: ", __FILE__, __LINE__);
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

	// lists assets
	void ListAllAssets() const {
		std::cout << "Asset Manager Currently Holds These Assets: " << std::endl;
		for (auto it = assetManager.begin(); it != assetManager.end(); it++) {
			std::cout << it->first.name << std::endl;
		}
		std::cout << "------------------------------------------" << std::endl;
	}

	// removes assets
	void RemoveAllAssets() {
		std::cout << "Deleting All Assets In Asset Manager" << std::endl;
		assetManager.clear();
	}

	bool OnCreate();

};


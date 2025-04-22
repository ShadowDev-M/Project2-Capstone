#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include "Debug.h"

#include "MaterialComponent.h"
#include "MeshComponent.h"
#include "ShaderComponent.h"

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
	std::unordered_map<std::string, Ref<Component>> assetManager;

public:

	// Meyers Singleton (from JPs class)
	static AssetManager& getInstance() {
		static AssetManager instance;
		return instance;
	}

	~AssetManager() { RemoveAllAssets(); }

	template<typename AssetTemplate, typename ... Args>
	bool AddAsset(std::string name_, Args&& ... args_) { 
		
		// check to see if theres already an asset in the assetmanager with the same name
		auto it = assetManager.find(name_);
		if (it != assetManager.end()) {
			Debug::Warning("Asset: " + name_ + "| already exists.", __FILE__, __LINE__);
			return false;
		}

		// add asset to assetmanager
		Ref<AssetTemplate> asset = std::make_shared<AssetTemplate>(std::forward<Args>(args_)...);
		assetManager[name_] = asset;
		return true;
	}

	template<typename AssetTemplate>
	Ref<AssetTemplate> GetAsset(const std::string& assetName_) const {
		auto it = assetManager.find(assetName_);

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
	
	// lists assets
	void ListAllAssets() const {
		std::cout << "Asset Manager Currently Holds These Assets: " << std::endl;
		for (auto it = assetManager.begin(); it != assetManager.end(); it++) {
			std::cout << it->first << std::endl;
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


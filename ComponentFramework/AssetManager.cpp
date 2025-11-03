#include "AssetManager.h"
#include <MMath.h>
#include <filesystem>

bool AssetManager::OnCreate() {
	std::cout << "Initialzing all assets: " << std::endl;

    if (std::filesystem::create_directory("Asset Manager"));

    // load all the assets from the asset database
    LoadAssetDatabaseXML();

    // when loading the assets from the database, all the assets' OnCreate are called, this is just extra insurance to call any of the assets already in the assetmanagers oncreate
    for (auto& asset : assetManager) {
        if (!asset.second->OnCreate()) {
            Debug::Error("Asset failed to initialize: " + asset.first.name, __FILE__, __LINE__);
            return false;
        }
    }

    return true;
}

bool AssetManager::SaveAssetDatabaseXML() const
{
    // create variable to xml doc
    XMLDocument doc;

    // create the root element for the xml file and insert it
    XMLNode* adbRoot = doc.NewElement("AssetDatabase");
    doc.InsertFirstChild(adbRoot);

    // using an std::map for better sorting of assets by their component type
    std::map<std::string, std::vector<std::pair<std::string, Ref<Component>>>> allAssetsByType;

    // [key.first/second] accesses the vector for the given key, if it doesn't exist it creates it
    for (const auto& asset : assetManager) {
        allAssetsByType[asset.first.componentType].push_back(std::make_pair(asset.first.name, asset.second));
    }

    // for each component type create a new component type element
    for (const auto& componentType : allAssetsByType) {
        XMLElement* componentTypeElement = doc.NewElement(componentType.first.c_str());

        // gets each asset's component type and creates a new element for it
        for (const auto& asset : componentType.second) {
            XMLElement* assetElement = doc.NewElement("Asset");
            // sets the name attribute to the name of the asset (i.e SM_Mario)
            assetElement->SetAttribute("name", asset.first.c_str());

            //TODO: if there are more components that need to go into the assetmanager later add them here

            // if statements to check for a specific component type
            // it checks to see if the component type from the asset matches the "", if yes, it gets its name and sets the filepath to it 
            if (componentType.first == "MeshComponent") {
                auto meshComponent = std::dynamic_pointer_cast<MeshComponent>(asset.second);
                if (meshComponent && meshComponent->getMeshName()) {
                    assetElement->SetAttribute("filepath", meshComponent->getMeshName());
                }
            }
            else if (componentType.first == "MaterialComponent") {
                auto materialComponent = std::dynamic_pointer_cast<MaterialComponent>(asset.second);
                if (materialComponent && materialComponent->getTextureName()) {
                    assetElement->SetAttribute("filepath", materialComponent->getTextureName());
                }
            }
            else if (componentType.first == "ShaderComponent") {
                auto shaderComponent = std::dynamic_pointer_cast<ShaderComponent>(asset.second);
                if (shaderComponent) {
                    if (shaderComponent->GetVertName()) {
                        assetElement->SetAttribute("vertShader", shaderComponent->GetVertName());
                    }
                    if (shaderComponent->GetFragName()) {
                        assetElement->SetAttribute("fragShader", shaderComponent->GetFragName());
                    }
                    // TODO: rest of the shader file types
                }
            }

            // insert all the assets to its own component type
            componentTypeElement->InsertEndChild(assetElement);
        }
        // insert all the component types to the root
        adbRoot->InsertEndChild(componentTypeElement);
    }

    // save all the assets to the database
    XMLError result = doc.SaveFile(("Asset Manager/" + assetDatabasePath + ".xml").c_str());
    if (result != XML_SUCCESS) {
        Debug::Error("Failed to save assets to database: " + assetDatabasePath, __FILE__, __LINE__);
        return false;
    }

    // debug info that prints to console showing that the saving of assets was succesful
    Debug::Info("Successfully saved " + std::to_string(assetManager.size()) + " assets to: " + assetDatabasePath + ".xml", __FILE__, __LINE__);
    return true;
}

bool AssetManager::LoadAssetDatabaseXML()
{
    XMLDocument doc;
    std::string fullAssetDBPath = "Asset Manager/" + assetDatabasePath + ".xml"; // possibly make into private variable instead of local

    // try to load the asset database xml file
    XMLError result = doc.LoadFile(fullAssetDBPath.c_str());
    if (result != XML_SUCCESS) {
        Debug::Error("Failed to load asset database: " + fullAssetDBPath, __FILE__, __LINE__);
        return false;
    }

    // get the root node for the xml file
    XMLNode* adbRoot = doc.RootElement();
    if (!adbRoot) {
        Debug::Error("No root element found in asset database: " + fullAssetDBPath, __FILE__, __LINE__);
        return false;
    }

    // some variables for debugging to keep track of how many assets get loaded and/or skipped
    int assetsLoaded = 0;
    int assetsSkipped = 0;

    // loop through each component type by accessing child elements
    for (XMLElement* componentTypeElement = adbRoot->FirstChildElement(); componentTypeElement != nullptr; componentTypeElement = componentTypeElement->NextSiblingElement()) {
        
        // get the name for the component type
        std::string componentType = componentTypeElement->Name();

        // loop through each asset of the current component type
        for (XMLElement* assetElement = componentTypeElement->FirstChildElement("Asset"); assetElement != nullptr; assetElement = assetElement->NextSiblingElement("Asset")) {
            
            // using const char* because checking for nullptr
            const char* assetName = assetElement->Attribute("name");
            if (!assetName) {
                Debug::Warning("Asset is missing name attribute.", __FILE__, __LINE__);
                continue;
            }

            // check to see if asset already exists
            AssetKey key = { std::string(assetName), componentType };
            if (assetManager.find(key) != assetManager.end()) {
                Debug::Info("Asset already exists, skipped: " + std::string(assetName) + ". Type:" + componentType, __FILE__, __LINE__);
                assetsSkipped++;
                continue;
            }

            // load the asset based on the component type
            bool success = false;
            if (componentType == "MeshComponent") {
                success = LoadAssetTypeXML<MeshComponent>(assetElement, assetName);
            }
            else if (componentType == "MaterialComponent") {
                success = LoadAssetTypeXML<MaterialComponent>(assetElement, assetName);
            }
            else if (componentType == "ShaderComponent") {
                success = LoadAssetTypeXML<ShaderComponent>(assetElement, assetName);
            }
            
            //TODO: if there are more components that need to go into the assetmanager later add them here

            else {
                Debug::Error("Unrecognized component type: " + componentType, __FILE__, __LINE__);
                continue;
            }

            // debug info
            if (success) {
                assetsLoaded++;
            }
        }
    }

    // debug info
    Debug::Info("Loaded " + std::to_string(assetsLoaded) + " assets, skipped " + std::to_string(assetsSkipped) + " existing assets", __FILE__, __LINE__);
    return true;
}

template<typename AssetTemplate>
bool AssetManager::LoadAssetTypeXML(XMLElement* assetElement_, const std::string& assetName_)
{
    // get the typeID for a compontent and return its name
    std::string componentType = static_cast<std::string>(typeid(AssetTemplate).name()).substr(6); //TODO: substr removes the 'class ' that gets added when getting the name from a typeid, 
                                                                                                  // this is compiler dependent, maybe figure out a way to improve this later

    // if constexpr allows for compile time branching, basically only branches that are true get compiled ##https://en.cppreference.com/w/cpp/language/if.html#Constexpr_if
    if constexpr (std::is_same_v<AssetTemplate, MeshComponent>) {
        // attribute assumes that "filepath" exists and returns its name(const char*) 
        const char* filepath = assetElement_->Attribute("filepath");
        if (!filepath) {
            Debug::Error(componentType + " missing filepath attribute: " + assetName_, __FILE__, __LINE__);
            return false;
        }

        // add the specific asset to the assetmanager + error handling (AddAsset already handles if there is a duplicate asset trying to be added)
        bool result = AddAsset<MeshComponent>(assetName_, nullptr, filepath);
        if (!result) {
            Debug::Error("Failed to add " + componentType + " to the Asset Manager: " + assetName_, __FILE__, __LINE__);
            return false;
        }

        // get the specific asset and call its oncreate + error handling
        auto asset = GetAsset<MeshComponent>(assetName_);
        if (!asset) {
            Debug::Error("Failed to retrieve " + componentType + "after adding: " + assetName_, __FILE__, __LINE__); // GetAsset has an error message for when an asset can't be found when trying to get it, 
            return false;                                                                                            // this is just extra insurance incase something else messes up
        }
        if (!asset->OnCreate()) {
            Debug::Error("Failed to initialize " + componentType + ". This asset's OnCreate failed: " + assetName_, __FILE__, __LINE__);
            RemoveAsset<MeshComponent>(assetName_);
            return false;
        }

        // if the asset manages to pass everything, then display an info debug message
        Debug::Info("Succesfully loaded " + componentType + ": " + assetName_ + " from: " + filepath, __FILE__, __LINE__);
        return true;
    }

    // same thing, this time for materials
    else if constexpr (std::is_same_v<AssetTemplate, MaterialComponent>) {
        // attribute assumes that "filepath" exists and returns its name(const char*) 
        const char* filepath = assetElement_->Attribute("filepath");
        if (!filepath) {
            Debug::Error(componentType + " missing filepath attribute: " + assetName_, __FILE__, __LINE__);
            return false;
        }

        // add the specific asset to the assetmanager + error handling (AddAsset already handles if there is a duplicate asset trying to be added)
        bool result = AddAsset<MaterialComponent>(assetName_, nullptr, filepath);
        if (!result) {
            Debug::Error("Failed to add " + componentType + " to the Asset Manager: " + assetName_, __FILE__, __LINE__);
            return false;
        }

        // get the specific asset and call its oncreate + error handling
        auto asset = GetAsset<MaterialComponent>(assetName_);
        if (!asset) {
            Debug::Error("Failed to retrieve " + componentType + "after adding: " + assetName_, __FILE__, __LINE__); // GetAsset has an error message for when an asset can't be found when trying to get it, 
            return false;                                                                                            // this is just extra insurance incase something else messes up
        }
        if (!asset->OnCreate()) {
            Debug::Error("Failed to initialize " + componentType + ". This asset's OnCreate failed: " + assetName_, __FILE__, __LINE__);
            RemoveAsset<MaterialComponent>(assetName_);
            return false;
        }

        // if the asset manages to pass everything, then display an info debug message
        Debug::Info("Succesfully loaded " + componentType + ": " + assetName_ + " from: " + filepath, __FILE__, __LINE__);
        return true;
    }

    else if constexpr (std::is_same_v<AssetTemplate, ShaderComponent>) {
        // attribute assumes that the path for shaders exists and returns its name(const char*) 
        const char* vertShader = assetElement_->Attribute("vertShader");
        const char* fragShader = assetElement_->Attribute("fragShader");

        // checks to see if any shaders are null
        if (!vertShader || !fragShader) {
            Debug::Error(componentType + " missing shader path attribute(s): " + assetName_, __FILE__, __LINE__);
            return false;
        }

        // add the specific asset to the assetmanager + error handling (AddAsset already handles if there is a duplicate asset trying to be added)
        bool result = AddAsset<ShaderComponent>(assetName_, nullptr, vertShader, fragShader);
        if (!result) {
            Debug::Error("Failed to add " + componentType + " to the Asset Manager: " + assetName_, __FILE__, __LINE__);
            return false;
        }

        // get the specific asset and call its oncreate + error handling
        auto asset = GetAsset<ShaderComponent>(assetName_);
        if (!asset) {
            Debug::Error("Failed to retrieve " + componentType + "after adding: " + assetName_, __FILE__, __LINE__); // GetAsset has an error message for when an asset can't be found when trying to get it, 
            return false;                                                                                            // this is just extra insurance incase something else messes up
        }
        if (!asset->OnCreate()) {
            Debug::Error("Failed to initialize " + componentType + ". This asset's OnCreate failed: " + assetName_, __FILE__, __LINE__);
            RemoveAsset<ShaderComponent>(assetName_);
            return false;
        }

        //TODO: rest of the shader types

        // if the asset manages to pass everything, then display an info debug message
        Debug::Info("Succesfully loaded " + componentType + ": " + assetName_ + " (vert: " + vertShader + ", frag: " + fragShader + ")", __FILE__, __LINE__);
        return true;
    }

    //TODO: if there are more components that need to go into the assetmanager later add them here

    else {
        // throw an error if the asset is unrecognized
        Debug::Error("Unrecognized component type: " + componentType, __FILE__, __LINE__);
        return false;
    }
}

bool AssetManager::ReloadAssetsXML()
{
    // if for some reason the xml file has assets that the local assetmanager doesnt
    RemoveAllAssets();
    return LoadAssetDatabaseXML();
}

template<typename AssetTemplate>
bool AssetManager::RemoveAsset(const std::string& assetName_)
{
    std::string componentType = static_cast<std::string>(typeid(AssetTemplate).name()).substr(6);
    AssetKey key = { assetName_, componentType };

    // finds the specific key and removes it
    auto it = assetManager.find(key);
    if (it != assetManager.end()) {
        it->second->OnDestroy();
        assetManager.erase(it);
        Debug::Info("Removed asset: " + assetName_ + " (" + componentType + ")", __FILE__, __LINE__);
        return true;
    }

    Debug::Warning("Attempted to remove non-existent asset: " + assetName_ + " (" + componentType + ")", __FILE__, __LINE__);
    return false;
}



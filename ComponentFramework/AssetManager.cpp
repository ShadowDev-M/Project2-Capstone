#include "AssetManager.h"
#include <MMath.h>

bool AssetManager::OnCreate() {
	std::cout << "Initialzing all assets: " << std::endl;

    // SM: (Static) Mesh
    AddAsset<MeshComponent>("SM_Mario", nullptr, "meshes/Mario.obj");

    AddAsset<MeshComponent>("SM_Sphere", nullptr, "meshes/Sphere.obj");
    AddAsset<MeshComponent>("SM_Plane", nullptr, "meshes/Plane.obj");
    AddAsset<MeshComponent>("SM_Pawn", nullptr, "meshes/Pawn.obj");

    // M: Material
    AddAsset<MaterialComponent>("M_MarioN", nullptr, "textures/mario_main.png");
    AddAsset<MaterialComponent>("M_Sphere", nullptr, "textures/Black Chess Base Colour.png");
    AddAsset<MaterialComponent>("M_ChessBoard", nullptr, "textures/8x8_checkered_board.png");

    // S: Shader
    AddAsset<ShaderComponent>("S_Phong", nullptr, "shaders/texturePhongVert.glsl", "shaders/texturePhongFrag.glsl");
    AddAsset<ShaderComponent>("S_Default", nullptr, "shaders/defaultVert.glsl", "shaders/defaultFrag.glsl");

    // if an asset was setup wrong throw an error
    for (auto& asset : assetManager) {
        if (!asset.second->OnCreate()) {
            Debug::Error("Asset failed to initialize: " + asset.first, __FILE__, __LINE__);
            return false;
        }
    }

    return true;
}
#pragma once
#include "imgui.h"
#include "imgui_stdlib.h"
#include "AssetManager.h"
#include "SceneGraph.h"

class AssetManagerWindow
{
	// delete the move and copy constructers
	AssetManagerWindow(const AssetManagerWindow&) = delete;
	AssetManagerWindow(AssetManagerWindow&&) = delete;
	AssetManagerWindow& operator = (const AssetManagerWindow&) = delete;
	AssetManagerWindow& operator = (AssetManagerWindow&&) = delete;
private:
	// filter used to search for specific assets
	ImGuiTextFilter assetFilter;
	
	// used for sizing asset layout
	const int thumbnailSize = 64;
	const int padding = 10;

	// I don't think the asset manager window has any calls to the scenegraph but for some reason the code just breaks without it
	SceneGraph* sceneGraph;

	// tracks dialog status
	bool showAddAssetDialog = false;

	// using imgui_stdlib for InputText(), these will help with passing on the names and paths of assets
	std::string newAssetName = "";
	std::string newAssetPath = "";
	std::string newDiffuseMapPath = "";
	std::string newSpecularMapPath = "";
	std::string newVertShaderPath = "";
	std::string newFragShaderPath = "";
	int selectedAssetType = 0; // 0 = Mesh, 1 = Material, 2 = Shader, etc

	//
	void ShowAddAssetDialog();

	void ResetInput();

	//
	bool AddNewAssetToDatabase();

	// TODO: possibly add this for file dialog https://github.com/aiekick/ImGuiFileDialog

	/// <summary>
	/// recursive function that gets called in showassetmanagerwindow to draw all the assets
	/// </summary>
	/// <param name="assetName"></param>
	/// <param name="asset"></param>
	void DrawAssetThumbnail(const std::string& assetName, Ref<Component> asset);
public:
	explicit AssetManagerWindow(SceneGraph* sceneGraph_);
	~AssetManagerWindow() {}

	void ShowAssetManagerWindow(bool* pOpen);

	/// <summary>
	/// gets called when in a scenes on destroy to make sure the filter is cleared 
	/// (just adding this incase we decide to use the same window for multiple scenes)
	/// </summary>
	void ClearFilter() { assetFilter.Clear(); }
};
#pragma once
#include "imgui.h"
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
	// text filter for the asset manager window
	ImGuiTextFilter assetFilter;
	
	// variables for asset sizes
	const int thumbnail_size = 64;
	const int padding = 10;

	SceneGraph* sceneGraph;

	void DrawAssetThumbnail(const std::string& assetName, Ref<Component> asset);
public:
	explicit AssetManagerWindow(SceneGraph* sceneGraph_);
	~AssetManagerWindow() {}

	void ShowAssetManagerWindow(bool* p_open);

	/// <summary>
	/// gets called when in a scenes on destroy to make sure the filter is cleared 
	/// (just adding this incase we decide to use the same window for multiple scenes)
	/// </summary>
	void ClearFilter() { assetFilter.Clear(); }
};
#pragma once

enum class AssetIcon {
	Folder, Mesh, Texture, Material, Shader, Script,
	Animation, Scene, Prefab, GlslFile, Unknown
};

class ProjectWindow {
	ProjectWindow(const ProjectWindow&) = delete;
	ProjectWindow(ProjectWindow&&) = delete;
	ProjectWindow& operator = (const ProjectWindow&) = delete;
	ProjectWindow& operator = (ProjectWindow&&) = delete;

	// window state variables
	fs::path selectedDir;
	fs::path selectedFile;
	ImGuiTextFilter filter;
	bool needsRefresh = true;

	bool isRenaming = false;
	fs::path renamePath;
	char renameBuffer[256] = {};

	// struct for what a typical file in the project window needs to have
	struct FileEntry {
		fs::path absolutePath;
		std::string displayName; // filename
		bool isDir = false;
		AssetIcon iconType = AssetIcon::Unknown;
		std::string assetName; // stem for assetmanager lookup
		std::string componentType;
	};
	std::vector<FileEntry> entries;
	std::vector<fs::path> folderTree;
	
	// entry helpers
	void RebuildEntries();
	void BuildFolderTree(const fs::path& root);
	AssetIcon GetIconFileEntry(const fs::directory_entry& e) const;

	// building out the window
	void DrawLeftPanel();
	void DrawRightPanel();
	void DrawFolderNode(const fs::path& dir);
	void DrawFileGrid();
	void DrawContextMenuEmpty();
	void DrawContextMenuFile(const FileEntry& entry);
	void DrawContextMenuFolder(const fs::path& dir);

	// filesystem functions
	void CreateFolder(const std::string& name);
	void CreateMatManifest(const std::string& name);
	void CreateMatFromTexture(const FileEntry& entry);
	void CreateShaderManifest(const std::string& name);
	void CreateScene(const std::string& name);
	void CreatePrefab(const std::string& name);
	void CreateScript(const std::string& name);
	void Rename(const fs::path& path, const std::string& newName);
	void Duplicate(const fs::path& path);
	void Delete(const fs::path& path);
	
	void MoveToFolder(const fs::path& filePath, const fs::path& destFolder);

	GLuint GetIconEntry(const FileEntry& e) const;

	const float TILE_SIZE = 72.0f; // 56
	const float PANEL_WIDTH = 180.0f;

public:
	explicit ProjectWindow();
	~ProjectWindow() = default;

	void ShowProjectWindowWindow(bool* pOpen);
	void NeedsRefresh() { needsRefresh = true; }
	void ClearFilter() { filter.Clear(); }
	void ClearSelectedFile() { selectedFile.clear(); }
};


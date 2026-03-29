#pragma once

// helper class that holds all the locations for where files should go
// this will also help later on if we setup CMake since we can easily seperate the different file paths

namespace fs = std::filesystem;

class SearchPath
{
	SearchPath() = default;
	SearchPath(const SearchPath&) = delete;
	SearchPath& operator=(const SearchPath&) = delete;

	// root folder
	fs::path root;

public:
	static SearchPath& getInstance() {
		static SearchPath instance;
		return instance;
	}

	void Initialize(const fs::path& assetsRoot);

	// takes a relative path like "meshes/Mario.obj" and converts it to an absolute path "C:\Users...\meshes\Mario.obj"
	fs::path Resolve(const fs::path& relative) const;

	// converts an absolute path to a relative path, used for things that need a relative path, like assets or manifests
	fs::path MakeRelative(const fs::path& absolute) const;

	// lists the entire directory of a relative directory, including folders and files
	std::vector<fs::directory_entry> ListDirectory(const fs::path& relDir) const;

	// takes a relative directory and returns all files that have the given extensions  ("Textures", { ".png", ".jpg", etc })
	std::vector<fs::path> FindByExtension(const fs::path& relDir, const std::vector<std::string>& extensions) const;

	// if a relative path exists
	bool Exists(const fs::path& relative) const;

	// makes sure a subfolder like Textures or Meshes etc, exist, if not it creates them
	fs::path EnsureSubfolder(const std::string& name) const;

	const fs::path& GetRoot() const { return root; }
};


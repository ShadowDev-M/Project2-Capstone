#include "pch.h"
#include "SearchPath.h"

void SearchPath::Initialize(const fs::path& assetsRoot)
{
	root = fs::absolute(assetsRoot);

	// default subfolders for assets
	static const char* assetFolders[] = { "Animations", "Audio", "Game Objects", "Icons",
		"Materials", "Meshes", "Prefabs", "Scenes", "Scripts", "Shaders", "Textures" 
	};

	for (auto& folder : assetFolders) {
		fs::create_directories(root / folder);
	}
}

fs::path SearchPath::Resolve(const fs::path& relative) const
{
	fs::path relPath = root / relative;
	if (fs::exists(relPath)) return fs::absolute(relPath);
	return {};
}

fs::path SearchPath::MakeRelative(const fs::path& absolute) const
{
	std::error_code ec;
	fs::path relPath = fs::relative(absolute, root, ec);
	return ec ? absolute : relPath;
}

std::vector<fs::directory_entry> SearchPath::ListDirectory(const fs::path& relDir) const
{
	std::vector<fs::directory_entry> entries;
	fs::path absDir = root / relDir;
	if (!fs::is_directory(absDir)) return entries;

	for (auto& entry : fs::directory_iterator(absDir)) {
		entries.push_back(entry);
	}

	return entries;
}

std::vector<fs::path> SearchPath::FindByExtension(const fs::path& relDir, const std::vector<std::string>& extensions) const
{
	std::vector<fs::path> results;
	fs::path absDir = root / relDir;
	if (!fs::is_directory(absDir)) return results;

	for (auto& entry : fs::recursive_directory_iterator(absDir)) {
		if (!entry.is_regular_file()) continue;
		std::string ext = entry.path().extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower); // makes sure that all extention types are lowercase
		for (auto& allowed : extensions) {
			if (ext == allowed) {
				results.push_back(entry.path());
				break;
			}
		}
	}

	return results;
}

bool SearchPath::Exists(const fs::path& relative) const
{
	return !Resolve(relative).empty();
}

fs::path SearchPath::EnsureSubfolder(const std::string& name) const
{
	fs::path path = root / name;
	fs::create_directories(path);
	return path;
}

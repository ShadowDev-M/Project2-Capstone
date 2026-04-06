#pragma once
#include "ProjectSettings.h"

class ProjectSettingsManager
{
	ProjectSettingsManager() = default;
	ProjectSettingsManager(const ProjectSettingsManager&) = delete;
	ProjectSettingsManager(ProjectSettingsManager&&) = delete;
	ProjectSettingsManager& operator = (const ProjectSettingsManager&) = delete;
	ProjectSettingsManager& operator = (ProjectSettingsManager&&) = delete;

public:
	static ProjectSettingsManager& getInstance() {
		static ProjectSettingsManager instance;
		return instance;
	}

	static constexpr const char* fileName = "project.settings";

	// project settings file functions
	bool Save(const std::string& filePath) const;
	bool Load(const std::string& filePath);
	bool SaveDefault() const;
	bool LoadDefault();
	bool Exists() const;

	ProjectSettings& Get() { return settings; }
	const ProjectSettings& Get() const { return settings; }

private:
	ProjectSettings settings;
};


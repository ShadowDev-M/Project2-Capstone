#pragma once

using namespace irrklang;
using namespace MATH;

class AudioManager
{
private:
	// deleting copy and move constructers, setting up singleton
	AudioManager() = default;
	AudioManager(const AudioManager&) = delete;
	AudioManager(AudioManager&&) = delete;
	AudioManager& operator=(const AudioManager&) = delete;
	AudioManager& operator=(AudioManager&&) = delete;

	// setting up irrklang 
	ISoundEngine* soundEngine = nullptr;
	float masterVolume = 1.0f;
	bool isInitialized = false;

public:
	// Meyers Singleton (from JPs class)
	static AudioManager& getInstance() {
		static AudioManager instance;
		return instance;
	}

	// irrklang engine mangement
	bool Initialize();
	~AudioManager();
	void Shutdown();

	// wrapper functions for irrklangs play2D and play3D functions
	ISound* Play2D(const std::string& filename_, bool loop = false, float volume = 1.0f);
	ISound* Play3D(const std::string& filename_, const Vec3& pos_, bool loop = false, float volume = 1.0f);
	
	// position managment
	void SetListenerPos(const Vec3& pos_, const Vec3& lookDir_, const Vec3& velocity_ = Vec3(0.0f, 0.0f, 0.0f), const Vec3& upVec_ = Vec3(0.0f, 1.0f, 0.0f));
	void SetSoundPos(ISound* sound_, const Vec3& pos_);

	// helper funtions
	void SetMasterVolume(float volume_) {
		masterVolume = std::clamp(volume_, 0.0f, 1.0f);
		soundEngine->setSoundVolume(masterVolume);
	}
	float GetMasterVolume() const { return masterVolume; }

	bool IsInitialized() const { return isInitialized; }
	ISoundEngine* GetSoundEngine() { return soundEngine; }
};


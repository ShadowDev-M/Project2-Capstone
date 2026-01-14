#include "AudioManager.h"
#include "Debug.h"

AudioManager::~AudioManager() { 
	Shutdown(); 
}

bool AudioManager::Initialize() {
	if (isInitialized) { 
		Debug::Warning("AudioManager already initialized!", __FILE__, __LINE__); 
		return true; 
	}

	soundEngine = createIrrKlangDevice(ESOD_AUTO_DETECT, ESEO_DEFAULT_OPTIONS);

	if (!soundEngine) { return false; }

	soundEngine->setSoundVolume(masterVolume);

	Debug::Info("AudioManager succesfully initalized", __FILE__, __LINE__);
	return isInitialized = true;
}

void AudioManager::Shutdown() {
	if (!isInitialized) { return; }

	if (soundEngine) {
		soundEngine->stopAllSounds();
		soundEngine->drop();
		soundEngine = nullptr;
	}

	isInitialized = false;
	Debug::Info("AudioManager succesfully shutdown", __FILE__, __LINE__);
}
ISound* AudioManager::Play2D(const std::string& filename_, bool loop, float volume)
{
	if (!isInitialized || !soundEngine) { 
		Debug::Error("AudioManager not initialized", __FILE__, __LINE__);
		return nullptr; 
	}
	
	ISound* sound = soundEngine->play2D(filename_.c_str(), loop, false, true);
	
	if (sound) {
		sound->setVolume(volume * masterVolume);
	}
	else {
		Debug::Warning("Failed to play2D sound: " + filename_, __FILE__, __LINE__);
	}

	return sound;
}

ISound* AudioManager::Play3D(const std::string& filename_, const Vec3& pos_, bool loop, float volume)
{
	if (!isInitialized || !soundEngine) {
		Debug::Error("AudioManager not initialized", __FILE__, __LINE__);
		return nullptr;
	}

	// converting to irrklang vec3
	vec3df pos(-pos_.x, pos_.y, pos_.z);

	ISound* sound = soundEngine->play3D(filename_.c_str(), pos, loop, false, true);

	if (sound) {
		sound->setVolume(volume * masterVolume);
		// lots of options to change for this, leaving as default for now
		sound->setMinDistance(2.5f);
	}
	else {
		Debug::Warning("Failed to play3D sound: " + filename_, __FILE__, __LINE__);
	}

	return sound;
}

void AudioManager::SetListenerPos(const Vec3& pos_, const Vec3& lookDir_, const Vec3& velocity_, const Vec3& upVec_)
{
	if (!isInitialized || !soundEngine) { return; }

	vec3df pos(-pos_.x, pos_.y, pos_.z);
	vec3df lookDir(-lookDir_.x, lookDir_.y, lookDir_.z);
	vec3df velocity(-velocity_.x, velocity_.y, velocity_.z);
	vec3df upVec(upVec_.x, upVec_.y, upVec_.z);

	soundEngine->setListenerPosition(pos, lookDir, velocity, upVec);
}

void AudioManager::SetSoundPos(ISound* sound_, const Vec3& pos_)
{
	if (!sound_) {
		return;
	}

	vec3df pos(-pos_.x, pos_.y, pos_.z);

	sound_->setPosition(pos);
}

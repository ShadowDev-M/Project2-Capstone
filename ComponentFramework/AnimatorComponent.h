#pragma once 
#include "Component.h"
#include "Actor.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/mesh.h"
#include "assimp/postprocess.h"
#include "Vector.h"

class AnimatorComponent;

struct BoneVertex {
	Vec3 position;
	Vec3 normal;
	Vec2 texCoords;
	// ... other data ...
	Vec4 boneIds;     // int[4] - bone indices (0-99)
	Vec4 boneWeights;  // float[4] - weights summing to 1.0
};
class MeshComponent;
class Skeleton;
struct Bone;

struct VectorKey {
	// in ticks
	double time; 
	Vec3 value;
};

struct QuatKey {
	// in ticks
	double time;        
	Quaternion value;
};

struct NodeAnim {
	std::string nodeName;          // bone / node name
	std::vector<VectorKey> posKeys;
	std::vector<QuatKey>   rotKeys;
	std::vector<VectorKey> scaleKeys;
};

///Asset in AssetManager for the animation data.
class Animation : public Component {

	
	friend class SceneGraph;
	Animation(const Animation&) = delete;
	Animation(Animation&&) = delete;
	Animation& operator = (const Animation&) = delete;
	Animation& operator = (Animation&&) = delete;

	std::string filename;
	std::string assetname;

	void LoadSkeleton(const char* filename);

	bool fullyLoaded = false;
	
	bool hierarchyLoaded = false;



	std::string name;
	double duration;               // in ticks
	double ticksPerSecond;
	std::vector<NodeAnim> channels;
public:

	float getDuration() { return duration; }
	float getTicksPerSecond() { return ticksPerSecond; }

	void BuildSkeletonHierarchy(Skeleton* targetSkeleton, MeshComponent* mesh);

	void LoadAnimation(const char* file);

	Animation(Component* parent, const char* filename_);
	~Animation();


	std::vector<Bone> getBonesAtTime(double time, MeshComponent* mesh);

	const char* getName() const { return assetname.c_str(); }

	const char* getFilename() const { return filename.c_str(); }


	bool queryLoadStatus() { return fullyLoaded; }

	bool InitializeAnimation();

	void SendAssetname(std::string assetname_);

	bool OnCreate() override;
	void OnDestroy() override;
	void Update(const float deltaTime) override;
	void Render() const;

	void calculatePose(double time, Skeleton* skeleton, std::vector<Matrix4>& output);

};



///Interface between the AnimatorComponent attached to an actor and the Animation asset
class AnimationClip {
	friend class SceneGraph;

	
	float clipLength = -1.0f;
	float startTime = 0.0f;
	float speedMult = 1.0f;
	float currentTime = 0.0f;

	bool loop = true;

	bool playing = false;

	Ref<Animation> animation;

public:
	/*AnimationClip(const AnimationClip&) = delete;
	AnimationClip(AnimationClip&&) = delete;
	AnimationClip& operator = (const AnimationClip&) = delete;
	AnimationClip& operator = (AnimationClip&&) = delete;*/

	bool getActiveState();

	AnimationClip();
	~AnimationClip();
	
	void setAnimationStr(const char* animation_);

	void setAnimation(Ref<Animation> animation_);

	float getStartTime() { return startTime; }

	float getSpeedMult() { return speedMult; }

	float getCurrentTime() { return currentTime; }

	float getClipLength() { 
		if (clipLength < 0.0f) {
			InitializeClipLength();
		}
		return clipLength; 
	}

	void setStartTime(float time_) {

		if (!(animation && animation->queryLoadStatus())) return;

		if (clipLength >= 0.0f && time_ <= clipLength) { 
			startTime = time_; 
			if (!(time_ >= startTime && time_ <= clipLength)) { currentTime = startTime; }
		}
	
	}

	///Sets the start time while ignoring if the animation is loaded or not, use at your own risk (Intended for save file loading)
	void setStartTimeRaw(float time_) {	startTime = time_;	}


	void setSpeedMult(float speed_) { speedMult = speed_; }

	void setCurrentTime(float time_) {
		if (!(animation && animation->queryLoadStatus())) return;

		if (time_ >= startTime && time_ <= clipLength) { currentTime = time_; }
	}

	std::string getAnimName() { 
		if (animation && animation->queryLoadStatus()) { return animation->getName(); }
		else { return "LOADING..."; }
	}

	const char* getAnimNameCStr() const {
		if (animation && animation->queryLoadStatus()) { return animation->getName(); }
		else { return "LOADING..."; }
	}

	std::string getAnimFilename() {
		if (animation && animation->queryLoadStatus()) { return animation->getFilename(); }
		else { return "LOADING..."; }
	}

	const char* getAnimFilenameCStr() const {
		if (animation && animation->queryLoadStatus()) { return animation->getFilename(); }
		else { return "LOADING..."; }
	}



	bool hasAnim() { return (bool)animation; }

	void setLooping(bool state) { loop = state; }
	bool getLoopingState() { return loop; }


	void displayDataTest(); 
	
	void Play() {

		playing = true;
	}

	bool isPlaying() { return playing; }


	float getCurrentTimeInFrames();

	static void updateClipTimes(float deltaTime);

	void InitializeClipLength();

	void StopPlaying() { playing = false; }

};


class AnimatorComponent : public Component {

	friend class SceneGraph;

	//Make SceneGraph friend class to allow SceneGraph to authorize/deauthorize an actor's usage of a script by adding/removing it to users
	//Also you could remove it if you wanted to disable it 
	friend class MeshComponent;

	AnimatorComponent(const AnimatorComponent&) = delete;
	AnimatorComponent(AnimatorComponent&&) = delete;
	AnimatorComponent& operator = (const AnimatorComponent&) = delete;
	AnimatorComponent& operator = (AnimatorComponent&&) = delete;

	MeshComponent* mesh = nullptr;

	Skeleton* skeleton = nullptr;
	std::vector<int> boneIds;
	std::vector<float> boneWeights;
	std::vector<Matrix4> finalBoneMatrices;  // Per-actor bone transforms

	AnimationClip activeClip = AnimationClip();


public:
	//Ref<AnimationClip> getBaseAsset() { return baseAsset; }
	AnimatorComponent(Component* parent, float startTime_ = 0.0f, float speedMult_ = 0.0f, bool loop_ = false, std::string assetname_ = "");
	virtual ~AnimatorComponent();

	AnimationClip getAnimationClip() { return activeClip; }
	void setAnimationClip(AnimationClip newClip) { activeClip = newClip; }

	void setAnimation(Ref<Animation> animation_) { activeClip.setAnimation(animation_); };

	float getStartTime() { return activeClip.getStartTime(); }

	float getSpeedMult() { return activeClip.getSpeedMult(); }

	float getCurrentTime() { return activeClip.getCurrentTime(); }

	float getClipLength() { return activeClip.getClipLength(); }

	void setStartTime(float time_) {activeClip.setStartTime(time_);}

	void setSpeedMult(float speed_) { activeClip.setSpeedMult(speed_); }

	void setCurrentTime(float time_) {activeClip.setCurrentTime(time_);}

	void setLooping(bool state) { activeClip.setLooping(state); }

	bool getLoopingState() { return activeClip.getLoopingState(); }

	std::string getAnimName() {return activeClip.getAnimName();}

	std::string getAnimFilename() { return activeClip.getAnimFilename(); }

	void PlayClip() { activeClip.Play(); }

	bool isPlaying() { return activeClip.isPlaying(); }

	void StopClip() { activeClip.StopPlaying(); }


	bool hasAnim() { return activeClip.hasAnim(); }

	void copySkeletalData();

	bool OnCreate();
	void OnDestroy();
	void Render() const;
	void Update(const float deltaTime);

	std::vector<std::string> getBoneVisualData();

	std::vector<std::string> getBoneWeightVisualData();

	std::string getSkeletonVisualData();

	static void queryAllAnimators(MeshComponent* caller);
	void displayDataTest() { activeClip.displayDataTest(); }
};


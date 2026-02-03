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

	const char* getName() const { return filename.c_str(); }

	bool queryLoadStatus() { return fullyLoaded; }

	bool InitializeAnimation();

	bool OnCreate() override;
	void OnDestroy() override;
	void Update(const float deltaTime) override;
	void Render() const;

	void calculatePose(double time, Skeleton* skeleton, std::vector<Matrix4>& output);

};



///Interface between the AnimatorComponent attached to an actor and the Animation asset
class AnimationClip {
	friend class SceneGraph;

	AnimationClip(const AnimationClip&) = delete;
	AnimationClip(AnimationClip&&) = delete;
	AnimationClip& operator = (const AnimationClip&) = delete;
	AnimationClip& operator = (AnimationClip&&) = delete;

	float clipLength = -1.0f;
	float currentTime = 0.0f;

	bool loop = true;

	bool playing = false;

	Ref<Animation> animation;

public:

	bool getActiveState();

	AnimationClip();
	~AnimationClip();

	
	void setAnimation(Ref<Animation> animation_);



	void displayDataTest(); 

	void Play() {
		playing = true;
	}

	float getCurrentTimeInFrames();

	static void updateClipTimes(float deltaTime);

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
	AnimatorComponent(Component* parent);
	virtual ~AnimatorComponent();

	void setAnimation(Ref<Animation> animation_) { activeClip.setAnimation(animation_); };

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


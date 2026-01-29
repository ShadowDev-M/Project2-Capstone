#pragma once 
#include "Component.h"
#include "Actor.h"

class AnimatorComponent;


class MeshComponent;
class Skeleton;
struct Bone;



///Asset in AssetManager for the animation data.
class Animation : public Component {
	Animation(const Animation&) = delete;
	Animation(Animation&&) = delete;
	Animation& operator = (const Animation&) = delete;
	Animation& operator = (Animation&&) = delete;

	std::string filename;

	void LoadSkeleton(const char* filename);

	bool fullyLoaded = false;

public:


	Animation(Component* parent, const char* filename_);
	~Animation();


	const char* getName() const { return filename.c_str(); }

	bool queryLoadStatus() { return fullyLoaded; }

	bool InitializeAnimation();

	bool OnCreate() override;
	void OnDestroy() override;
	void Update(const float deltaTime) override;
	void Render() const;

};


///Interface between the AnimatorComponent attached to an actor and the Animation asset
class AnimationClip {
	AnimationClip(const AnimationClip&) = delete;
	AnimationClip(AnimationClip&&) = delete;
	AnimationClip& operator = (const AnimationClip&) = delete;
	AnimationClip& operator = (AnimationClip&&) = delete;

	float clipLength = 0.0f;
	float currentTime = 0.0f;

	Ref<Animation> animation;

public:

	AnimationClip();
	~AnimationClip();

	void setAnimation(Ref<Animation> animation_);



};


class AnimatorComponent : public Component {


	//Make SceneGraph friend class to allow SceneGraph to authorize/deauthorize an actor's usage of a script by adding/removing it to users
	//Also you could remove it if you wanted to disable it 
	friend class MeshComponent;

	AnimatorComponent(const AnimatorComponent&) = delete;
	AnimatorComponent(AnimatorComponent&&) = delete;
	AnimatorComponent& operator = (const AnimatorComponent&) = delete;
	AnimatorComponent& operator = (AnimatorComponent&&) = delete;

	MeshComponent* mesh = nullptr;

	Skeleton* skeleton = nullptr;
	std::vector<float> boneIds;
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

};
#pragma once 
#include "Component.h"
#include "Actor.h"

class AnimatorComponent;


class MeshComponent;
class Skeleton;
struct Bone;


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
	float animationTime = 0.0f;

public:
	//Ref<AnimationClip> getBaseAsset() { return baseAsset; }
	AnimatorComponent(Component* parent);
	virtual ~AnimatorComponent();

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

#include "pch.h"

#include "AnimatorComponent.h"
#include "MeshComponent.h"
#include "Skeleton.h"
#include "SceneGraph.h"
#include <vector>
#include <algorithm>
#include "MMath.h"
#include <iostream>
#include <algorithm>
#include "AssetManager.h"
static std::vector<AnimatorComponent*> animators;

void AnimatorComponent::queryAllAnimators(MeshComponent* caller)
{
	for (auto& animator : animators) {
		if (animator->mesh == caller) {
			animator->copySkeletalData();
		}
	}
}

AnimatorComponent::AnimatorComponent(Component* parent_, float startTime_, float speedMult_, bool loop_, std::string assetname_ ) : Component(parent_)
{
	animators.push_back(this);

	if (parent) {

		Actor* user = dynamic_cast<Actor*>(parent);

		if (user) {

			auto meshShared = user->GetComponent<MeshComponent>();  
			mesh = meshShared.get();

			std::cout << mesh->getMeshName() << std::endl;
		}
		copySkeletalData();

		Ref<Animation> animationClip = std::dynamic_pointer_cast<Animation>(AssetManager::getInstance().GetAsset<Animation>(assetname_));

		if (animationClip)
			activeClip.setAnimation(animationClip);

		activeClip.setStartTimeRaw(startTime_);
		activeClip.setSpeedMult(speedMult_);
		activeClip.setLooping(loop_);
		
		
	}
	else {
#ifdef _DEBUG
		Debug::Error("Attempted to create AnimatorComponent for null actor", __FILE__, __LINE__);
#endif
	}


}

AnimatorComponent::~AnimatorComponent()
{

	auto it = std::find(animators.begin(), animators.end(), this);
	if (it != animators.end()) {
		animators.erase(it);
	}
}

void AnimatorComponent::copySkeletalData()
{


		if (mesh && mesh->skeleton) {
			skeleton = mesh->skeleton.get();
			boneIds = mesh->boneIds;
			boneWeights = mesh->boneWeights;

			finalBoneMatrices.resize(skeleton->bones.size());
		}
	
	
}

bool AnimatorComponent::OnCreate()
{


	return true;
}

void AnimatorComponent::OnDestroy()
{
}

void AnimatorComponent::Render() const
{
}

void AnimatorComponent::Update(const float deltaTime) {
	

}

std::vector<std::string> AnimatorComponent::getBoneVisualData() {
	std::vector<std::string> bonesVisualData;
	for (size_t i = 0; i < skeleton->bones.size(); i++) {
		std::string visualDataString;
		const Bone* bone = skeleton->bones[i].get();
		visualDataString += "Bone[" + std::to_string(bone->id) + "] \"" + bone->name + "\"\n";
		visualDataString += "  Parent: " + std::string(bone->parent ? bone->parent->name : "none") + "\n";
		visualDataString += "  Children count: " + std::to_string(bone->children.size()) + "\n";
		visualDataString += "  Offset matrix:\n";

		char matrixLine[128];
		for (int row = 0; row < 4; row++) {
			snprintf(matrixLine, sizeof(matrixLine), "    %.3f %.3f %.3f %.3f\n",
				bone->offsetMatrix[row * 4 + 0],
				bone->offsetMatrix[row * 4 + 1],
				bone->offsetMatrix[row * 4 + 2],
				bone->offsetMatrix[row * 4 + 3]);
			visualDataString += matrixLine;
		}
		visualDataString += "\n";

		bonesVisualData.push_back(visualDataString);

	}
	return bonesVisualData;
}

std::vector<std::string> AnimatorComponent::getBoneWeightVisualData() {
	std::vector<std::string> bonesVisualData;
	int numVertsToPrint = std::min(5, (int)mesh->boneIds.size() / 4);
	for (int v = 0; v < numVertsToPrint; v++) {
		std::string visualDataString;
		int offset = v * 4;
		char vertexLine[256];
		snprintf(vertexLine, sizeof(vertexLine), "Vertex %d: ", v);
		visualDataString += vertexLine;

		for (int k = 0; k < 4; k++) {
			snprintf(vertexLine, sizeof(vertexLine), "B%d(%.2f) ",
				(int)mesh->boneIds[offset + k], mesh->boneWeights[offset + k]);
			visualDataString += vertexLine;
		}
		visualDataString += "\n";


		bonesVisualData.push_back(visualDataString);
	}

	return bonesVisualData;
}
std::string AnimatorComponent::getSkeletonVisualData()
{

	std::string visualDataString = R"(
	Skeleton Debug
)";

	visualDataString += "Bones count: " + std::to_string(skeleton->bones.size()) + "\n\n";
	return visualDataString;
	// Print all bones
	

	// Print first few vertex bone weights
	
}

bool AnimationClip::getActiveState()
{
	if (animation &&
		animation->queryLoadStatus() && playing) {
		return true;
	}

	return false;
}

static std::vector<AnimationClip*> clipsToUpdate;

AnimationClip::AnimationClip()
{
	clipsToUpdate.push_back(this);
}

AnimationClip::~AnimationClip()
{
	clipsToUpdate.erase(std::remove(clipsToUpdate.begin(), clipsToUpdate.end(), this), clipsToUpdate.end());
}

void AnimationClip::setAnimationStr(const char* animation_)
{ setAnimation(AssetManager::getInstance().GetAsset<Animation>(std::string(animation_))); }



void AnimationClip::setAnimation(Ref<Animation> animation_)
{

	if (!animation_) return;

	startTime = 0.0f;
	if (animation_ && !animation_->queryLoadStatus()) {
		SceneGraph::getInstance().pushAnimationToWorker(animation_);
	}
	animation = animation_;

}

void Animation::preloadAnimation(std::string animationName)
{
	Ref<Animation> animation_ = AssetManager::getInstance().GetAsset<Animation>(animationName);

	if (!animation_) return;

	if (animation_ && !animation_->queryLoadStatus()) {
		SceneGraph::getInstance().pushAnimationToWorker(animation_);
	}

}

void AnimationClip::displayDataTest()
{
	
	/*for (auto& obj : animation->getBonesAtTime(0)) {
		obj.offsetMatrix.print();
	}*/
}


float AnimationClip::getCurrentTimeInFrames()
{
	
	double ticksPerSecond = animation->getTicksPerSecond();
	double timeInTicks = currentTime * ticksPerSecond;
	return timeInTicks;
}

void AnimationClip::updateClipTimes(float deltaTime)
{
	for (auto& clip : clipsToUpdate) {
		if (clip->playing) {
			if (clip->animation->queryLoadStatus()) {
				if (clip->clipLength < 0.0f) {

					clip->clipLength = clip->animation->getDuration() / clip->animation->getTicksPerSecond();

				}

				clip->currentTime += deltaTime * clip->speedMult;

				if (clip->currentTime > clip->clipLength) {
					if (clip->loop)
						clip->currentTime = clip->startTime;
					else {
						clip->currentTime = clip->clipLength;
						clip->StopPlaying();
					}

				}
				else if (clip->currentTime < clip->startTime) {
					if (clip->loop) clip->currentTime = clip->clipLength;
					else {
						clip->currentTime = clip->startTime;
						clip->StopPlaying();
					}

				}

			}
		}
	}
}

void AnimationClip::InitializeClipLength() {
	if (animation && animation->queryLoadStatus()) {

		if (clipLength < 0.0f) {

			clipLength = animation->getDuration() / animation->getTicksPerSecond();

		}
	}
}

void Animation::LoadAnimation(const char* filename) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(
		filename,
		aiProcessPreset_TargetRealtime_Fast
	);

	if (!scene || !scene->HasAnimations()) {
	//	printf("No animations found!\n");
		return;
	}

	aiAnimation* anim = scene->mAnimations[0];
	name = anim->mName.C_Str();
	duration = anim->mDuration;
	ticksPerSecond = (anim->mTicksPerSecond != 0.0) ? anim->mTicksPerSecond : 25.0;

	channels.clear();
	channels.reserve(anim->mNumChannels);

	for (unsigned int i = 0; i < anim->mNumChannels; ++i) {
		aiNodeAnim* aiChan = anim->mChannels[i];
		NodeAnim chan;
		chan.nodeName = aiChan->mNodeName.C_Str();

		// position
		chan.posKeys.reserve(aiChan->mNumPositionKeys);
		for (unsigned int k = 0; k < aiChan->mNumPositionKeys; ++k) {
			VectorKey key;
			key.time = aiChan->mPositionKeys[k].mTime;
			key.value = Vec3(aiChan->mPositionKeys[k].mValue.x, aiChan->mPositionKeys[k].mValue.y, aiChan->mPositionKeys[k].mValue.z);
			chan.posKeys.push_back(key);
		}

		// rotation
		chan.rotKeys.reserve(aiChan->mNumRotationKeys);
		for (unsigned int k = 0; k < aiChan->mNumRotationKeys; ++k) {
			QuatKey key;
			key.time = aiChan->mRotationKeys[k].mTime;
			key.value = Quaternion(aiChan->mRotationKeys[k].mValue.w,
				Vec3(aiChan->mRotationKeys[k].mValue.x,
					aiChan->mRotationKeys[k].mValue.y,
					aiChan->mRotationKeys[k].mValue.z));
			chan.rotKeys.push_back(key);

			
		}

		// scale
		chan.scaleKeys.reserve(aiChan->mNumScalingKeys);
		for (unsigned int k = 0; k < aiChan->mNumScalingKeys; ++k) {
			VectorKey key;
			key.time = aiChan->mScalingKeys[k].mTime;
			key.value = Vec3(aiChan->mScalingKeys[k].mValue.x, aiChan->mScalingKeys[k].mValue.y, aiChan->mScalingKeys[k].mValue.z);
			chan.scaleKeys.push_back(key);
		}

		channels.push_back(std::move(chan));
	}

	

	fullyLoaded = true;
}
Animation::Animation(Component* parent, const char* filename_) : Component(nullptr)/*shouldn't really have a parent*/, filename(filename_)
{
}

Animation::~Animation()
{
}
struct KeyframeIndex {
	int index;  
	float frac; 
};

KeyframeIndex findKeyframe(const std::vector<VectorKey>& keys, double time) {
	KeyframeIndex result = { -1, 0.0f };
	if (keys.empty()) return result;

	if (keys.size() == 1 || time <= keys[0].time) {
		result.index = 0;
		return result;
	}

	for (size_t i = 0; i < keys.size() - 1; ++i) {
		if (time < keys[i + 1].time) {
			result.index = i;  
			double t0 = keys[i].time;
			double t1 = keys[i + 1].time;
			result.frac = (time - t0) / (t1 - t0);
			result.frac = std::clamp(result.frac, 0.0f, 1.0f);
			return result;
		}
	}
	result.index = keys.size() - 1;
	return result;
}

KeyframeIndex findKeyframe(const std::vector<QuatKey>& keys, double time) {
	KeyframeIndex result = { -1, 0.0f };
	if (keys.empty()) return result;

	if (keys.size() == 1 || time <= keys[0].time) {
		result.index = 0;
		return result;
	}

	for (size_t i = 0; i < keys.size() - 1; ++i) {
		if (time < keys[i + 1].time) {
			result.index = i;
			double t0 = keys[i].time;
			double t1 = keys[i + 1].time;
			result.frac = (time - t0) / (t1 - t0);
			return result;
		}
	}
	result.index = keys.size() - 1;
	return result;
}



void Animation::calculatePose(double time, Skeleton* skeleton, std::vector<Matrix4>& output) {
	Matrix4 identity;
	Matrix4 globalInverseTransform = skeleton->globalInverseTransform;

	std::function<void(Bone*, const Matrix4&)> computePose =
	[&](Bone* bone, const Matrix4& parentTransform) {

		NodeAnim* channel = nullptr;
		for (auto& chan : channels) {
			if (chan.nodeName == bone->name) {
				channel = &chan;
				break;
			}
		}

		// Interpolate position
		Vec3 position(0, 0, 0);
		if (channel && !channel->posKeys.empty()) {
			KeyframeIndex fp = findKeyframe(channel->posKeys, time);
			if (fp.index >= 0) {
				if (fp.index < channel->posKeys.size() - 1) {
					position = VMath::lerp(channel->posKeys[fp.index].value,
						channel->posKeys[fp.index + 1].value, fp.frac);
				}
				else {
					position = channel->posKeys[fp.index].value;
				}
			}

		}

		// Interpolate rotation
		Quaternion rotation(1, Vec3(0, 0, 0));
		if (channel && !channel->rotKeys.empty()) {
			KeyframeIndex fp = findKeyframe(channel->rotKeys, time);
			if (fp.index >= 0) {
				if (fp.index < channel->rotKeys.size() - 1) {
					rotation = QMath::slerp(channel->rotKeys[fp.index].value,
						channel->rotKeys[fp.index + 1].value, fp.frac);
				}
				else {
					rotation = channel->rotKeys[fp.index].value;
				}
			}
		}

		// Interpolate scale
		Vec3 scaleVec(1, 1, 1);
		if (channel && !channel->scaleKeys.empty()) {
			KeyframeIndex fp = findKeyframe(channel->scaleKeys, time);
			if (fp.index >= 0) {
				if (fp.index < channel->scaleKeys.size() - 1) {
					scaleVec = VMath::lerp(channel->scaleKeys[fp.index].value,
						channel->scaleKeys[fp.index + 1].value, fp.frac);
				}
				else {
					scaleVec = channel->scaleKeys[fp.index].value;
				}
			}
		}

		Matrix4 positionMat = MMath::translate(position);
		Matrix4 rotationMat = MMath::toMatrix4(rotation);
		Matrix4 scaleMat = MMath::scale(scaleVec);
		Matrix4 localTransform = positionMat * rotationMat * scaleMat;
		Matrix4 globalTransform = parentTransform * localTransform;
		
		if (!bone->name.empty() && bone->id >= 0 && bone->id < output.size()) {
			output[bone->id] = globalTransform * bone->offsetMatrix;

		}
		


		// Recurse children
		for (Bone* child : bone->children) {
			computePose(child, globalTransform);
		}
	};

	// Start from root bones
	for (auto& bonePtr : skeleton->bones) {
		if (!bonePtr->parent) {
			computePose(bonePtr.get(), identity);
		}
	}
}

bool Animation::InitializeAnimation()
{

	LoadAnimation(filename.c_str());

	return true;
}


void Animation::SendAssetname(std::string assetname_) {
	assetname = assetname_;
}

bool Animation::OnCreate()
{

	return true;
}

void Animation::OnDestroy()
{
}

void Animation::Update(const float deltaTime)
{
}

void Animation::Render() const
{
}


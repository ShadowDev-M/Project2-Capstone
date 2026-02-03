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
static std::vector<AnimatorComponent*> animators;

void AnimatorComponent::queryAllAnimators(MeshComponent* caller)
{
	for (auto& animator : animators) {
		if (animator->mesh == caller) {
			animator->copySkeletalData();
		}
	}
}

AnimatorComponent::AnimatorComponent(Component* parent_) : Component(parent_)
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
			// Reference the shared skeleton (not copy)
			skeleton = mesh->skeleton.get();
			boneIds = mesh->boneIds;
			boneWeights = mesh->boneWeights;

			// Pre-allocate bone matrices for this animator
			finalBoneMatrices.resize(skeleton->bones.size());

			std::cout << "SUCCESSFUL COPY INTO ANIMATOR" << std::endl;


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




void AnimationClip::setAnimation(Ref<Animation> animation_)
{
	if (animation_ && !animation_->queryLoadStatus()) {
		SceneGraph::getInstance().pushAnimationToWorker(animation_.get());
	}
	animation = animation_;

}

void AnimationClip::displayDataTest()
{
	
	/*for (auto& obj : animation->getBonesAtTime(0)) {
		obj.offsetMatrix.print();
	}*/
}


float AnimationClip::getCurrentTimeInFrames()
{
	if (currentTime >= clipLength) {
		if (loop) currentTime = 0;
		else {
			currentTime = clipLength;
			StopPlaying();
		}
		
	}
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

				clip->currentTime += deltaTime;
			}
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
		printf("No animations found!\n");
		return;
	}

	// Load animation channels (your existing code)
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

		// Positions (your existing code - unchanged)
		chan.posKeys.reserve(aiChan->mNumPositionKeys);
		for (unsigned int k = 0; k < aiChan->mNumPositionKeys; ++k) {
			VectorKey key;
			key.time = aiChan->mPositionKeys[k].mTime;
			key.value = Vec3(aiChan->mPositionKeys[k].mValue.x, aiChan->mPositionKeys[k].mValue.y, aiChan->mPositionKeys[k].mValue.z);
			chan.posKeys.push_back(key);
		}

		// Rotations (your existing code - unchanged)
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

		// Scales (your existing code - unchanged)
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
	printf("Animation loaded with %zu channels\n", channels.size());
}
Animation::Animation(Component* parent, const char* filename_) : Component(nullptr)/*shouldn't really have a parent*/, filename(filename_)
{
}

Animation::~Animation()
{
}
struct KeyframeIndex {
	int index;  // Start index for interpolation (what find*KeyIndex returned)
	float frac; // Interpolation factor [0,1]
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
			result.index = i;  // Use i (not i+1 like your old find*KeyIndex)
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






///deprecated 
std::vector<Bone> Animation::getBonesAtTime(double time, MeshComponent* mesh) {
	std::vector<Bone> bonesAtFrame(mesh->skeleton->bones.size());

	// Recursive function matching your reference exactly
	std::function<void(Bone*, const Matrix4&, std::unordered_map<std::string, Matrix4>&)> computeGlobalPose =
		[&](Bone* meshBone, const Matrix4& parentGlobal, std::unordered_map<std::string, Matrix4>& globalTransforms) {

		// Find animation channel
		NodeAnim* channel = nullptr;
		for (auto& chan : channels) {
			if (chan.nodeName == meshBone->name) {
				channel = &chan;
				break;
			}
		}

		// Sample LOCAL transform (exactly like your reference)
		Vec3 pos(0, 0, 0);
		Quaternion rot(1, Vec3(0, 0, 0));
		Vec3 scale(1, 1, 1);

		if (channel) {
			// POSITION
			if (!channel->posKeys.empty()) {
				KeyframeIndex fp = findKeyframe(channel->posKeys, time);
				if (fp.index >= 0) {
					if (fp.index < channel->posKeys.size() - 1) {
						pos = VMath::lerp(channel->posKeys[fp.index].value,
							channel->posKeys[fp.index + 1].value, fp.frac);
					}
					else {
						pos = channel->posKeys[fp.index].value;
					}
				}
			}

			// ROTATION - ARM TEST OVERRIDE
			if (!channel->rotKeys.empty()) {
				KeyframeIndex fp = findKeyframe(channel->rotKeys, time);
				if (fp.index >= 0) {
					if (fp.index < channel->rotKeys.size() - 1) {
						rot = QMath::normalize(QMath::slerp(channel->rotKeys[fp.index].value,
							channel->rotKeys[fp.index + 1].value, fp.frac));
					}
					else {
						rot = QMath::normalize(channel->rotKeys[fp.index].value);
					}
				}
			}

			// SCALE
			if (!channel->scaleKeys.empty()) {
				KeyframeIndex fp = findKeyframe(channel->scaleKeys, time);
				if (fp.index >= 0) {
					if (fp.index < channel->scaleKeys.size() - 1) {
						scale = VMath::lerp(channel->scaleKeys[fp.index].value,
							channel->scaleKeys[fp.index + 1].value, fp.frac);
					}
					else {
						scale = channel->scaleKeys[fp.index].value;
					}
				}
			}
		}

		// Build LOCAL transform (EXACTLY like your reference)
		Matrix4 positionMat = MMath::translate(pos);
		Matrix4 rotationMat = MMath::toMatrix4(rot);
		Matrix4 scaleMat = MMath::scale(scale);
		Matrix4 localTransform = positionMat * rotationMat * scaleMat;

		// Compute GLOBAL transform
		Matrix4 globalTransform = parentGlobal * localTransform;

		// Store final skinning matrix (NEED meshBone->offset here - assuming it's available)
		globalTransforms[meshBone->name] = globalTransform;

		// Recurse children (exactly like your reference)
		for (Bone* child : meshBone->children) {
			computeGlobalPose(child, globalTransform, globalTransforms);
		}
		};

	// Start from root bones
	std::unordered_map<std::string, Matrix4> globalTransforms;
	for (auto& bonePtr : mesh->skeleton->bones) {
		if (!bonePtr->parent) {
			computeGlobalPose(bonePtr.get(), Matrix4(), globalTransforms);
		}
	}

	// Build output bones
	for (size_t i = 0; i < mesh->skeleton->bones.size(); ++i) {
		auto& meshBonePtr = mesh->skeleton->bones[i];
		Bone& bone = bonesAtFrame[i];

		bone.name = meshBonePtr->name;
		bone.id = static_cast<int>(i);

		auto it = globalTransforms.find(meshBonePtr->name);
		bone.offsetMatrix = it != globalTransforms.end() ? it->second : Matrix4();

		// Hierarchy references
		if (meshBonePtr->parent) {
			for (size_t j = 0; j < mesh->skeleton->bones.size(); ++j) {
				if (mesh->skeleton->bones[j].get() == meshBonePtr->parent) {
					bone.parent = &bonesAtFrame[j];
					break;
				}
			}
		}

		bone.children.clear();
		for (Bone* child : meshBonePtr->children) {
			for (size_t j = 0; j < mesh->skeleton->bones.size(); ++j) {
				if (mesh->skeleton->bones[j].get() == child) {
					bone.children.push_back(&bonesAtFrame[j]);
					break;
				}
			}
		}
	}

	return bonesAtFrame;
}



void Animation::calculatePose(double time, Skeleton* skeleton, std::vector<Matrix4>& output) {
	Matrix4 identity;
	Matrix4 globalInverseTransform = skeleton->globalInverseTransform;

	std::function<void(Bone*, const Matrix4&)> computePose =
		[&](Bone* bone, const Matrix4& parentTransform) {

		// Find NodeAnim channel
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

		// EXACT reference: localTransform = position * rotation * scale
		Matrix4 positionMat = MMath::translate(position);
		Matrix4 rotationMat = MMath::toMatrix4(rotation);
		Matrix4 scaleMat = MMath::scale(scaleVec);
		Matrix4 localTransform = positionMat * rotationMat * scaleMat;
		Matrix4 globalTransform = parentTransform * localTransform;
		
		// EXACT reference final skinning matrix
		if (!bone->name.empty() && bone->id >= 0 && bone->id < output.size()) {
			output[bone->id] = globalTransform * bone->offsetMatrix;

			// DEBUG - PRINT FIRST 10 BONES ONLY
			/*std::cout << "globalTransform\n";
			globalTransform.print();
			std::cout << "offset\n";
			bone->offsetMatrix.print();
			std::cout << "output\n";
			output[bone->id].print();*/
		}
		else {
			printf("SKIPPED bone '%s' id=%d (size=%zu)\n", bone->name.c_str(), bone->id, output.size());
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


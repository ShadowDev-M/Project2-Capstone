#include "pch.h"
#include "SceneGraph.h"
#include "XMLManager.h"
#include "InputManager.h"
#include "AnimatorComponent.h"
#include "Skeleton.h"
#include "HierarchyWindow.h"
#include "EditorManager.h"
#include "PhysicsSystem.h"
#include "CollisionSystem.h"
#include "ScreenManager.h"
#include "LightingSystem.h"

SceneGraph::SceneGraph()
{
	ScriptService::loadLibraries();
}

SceneGraph::~SceneGraph()
{
	OnDestroy();
}

bool SceneGraph::OnCreate()
{
	// if an actor was setup wrong throw an error
	for (auto& actor : Actors) {
		if (!actor.second->OnCreate()) {
#ifdef _DEBUG
			Debug::Error("Actor failed to initialize: " + actor.second->getActorName(), __FILE__, __LINE__);
#endif
			return false;
		}
	}

	return true;
}

void SceneGraph::OnDestroy() {
	Stop();
	
	RemoveAllActors();
}

bool SceneGraph::AddActor(Ref<Actor> actor)
{
	if (!actor) {
#ifdef _DEBUG
		Debug::Error("Attempted to add null actor", __FILE__, __LINE__);
#endif
		return false;
	}

	const std::string& name = actor->getActorName();
	uint32_t id = actor->getId();

	// check if an actor with this ID already exists
	auto it = Actors.find(id);
	if (it != Actors.end()) {
		Debug::Warning("An actor with ID " + std::to_string(id) + " already exists", __FILE__, __LINE__);
		return false;
	}

	// check to see if there is already an actor with the same name in the map
	auto nameIt = ActorNameToId.find(name);
	if (nameIt != ActorNameToId.end() && nameIt->second != id) {
		Debug::Warning("An actor named: " + name + " already exists", __FILE__, __LINE__);
		return false;
	}

	// add the actor using ID as key
	Actors[id] = actor;
	ActorNameToId[name] = id;
	
	EditorManager::getInstance().UpdateActorHierarchy();

	return true;
}

void SceneGraph::Start()
{
	for (auto& actor : Actors) {
		ScriptService::startActorScripts(actor.second);
	}
}

void SceneGraph::Stop()
{
	for (auto& actor : Actors) {
		ScriptService::stopActorScripts(actor.second);

		Ref<AnimatorComponent> actorAnimator =  actor.second->GetComponent<AnimatorComponent>();
		if (actorAnimator) {
			actorAnimator->activeClip.StopPlaying();
			actorAnimator->activeClip.currentTime = 0.0f;
		}
	}

	ScriptService::ClearLuaState();

	// Stop physics engine 
	PhysicsSystem::getInstance().ResetPhysics();
}

void SceneGraph::LoadActor(const char* name_, Ref<Actor> parent) {
	
	Ref<Actor> actor_ = std::make_shared<Actor>(parent.get(), name_);

	// if statements to check whether or not a specific component exists
	// added this because before the engine would crash because it would be trying to add an actor that didn't have a mesh/material/shader
	if (XMLObjectFile::hasComponent<MaterialComponent>(name_)) {
		std::string materialName = XMLObjectFile::getComponent<MaterialComponent>(name_);
		if (!materialName.empty()) {
			Ref<MaterialComponent> materialComponent = AssetManager::getInstance().GetAsset<MaterialComponent>(materialName);
			if (materialComponent) {
				actor_->ReplaceComponent<MaterialComponent>(materialComponent);

				if (XMLObjectFile::hasComponent<TilingSettings>(name_)) {
					Ref<TilingSettings> TSC = Ref<TilingSettings>(std::apply([](auto&&... args) {
						return std::make_shared<TilingSettings>(args...);
						}, std::tuple_cat(std::make_tuple(actor_.get()), XMLObjectFile::getComponent<TilingSettings>(name_))));

					if (!actor_->GetComponent<TilingSettings>()) {
						actor_->AddComponent(TSC);
					}
				}
			}
		}
	} 
	// make sure that every actor with material gets a tiling setting defaulted to off
	if (actor_->GetComponent<MaterialComponent>() && !actor_->GetComponent<TilingSettings>())
		actor_->AddComponent<TilingSettings>(actor_.get(), false);

	if (XMLObjectFile::hasComponent<ShaderComponent>(name_)) {
		std::string shaderName = XMLObjectFile::getComponent<ShaderComponent>(name_);
		if (!shaderName.empty()) {
			Ref<ShaderComponent> shaderComponent = AssetManager::getInstance().GetAsset<ShaderComponent>(shaderName);
			if (shaderComponent) {
				actor_->ReplaceComponent<ShaderComponent>(shaderComponent);
			}
		}
	}

	if (XMLObjectFile::hasComponent<MeshComponent>(name_)) {
		std::string meshName = XMLObjectFile::getComponent<MeshComponent>(name_);
		if (!meshName.empty()) {
			Ref<MeshComponent> meshComponent = AssetManager::getInstance().GetAsset<MeshComponent>(meshName);
			if (meshComponent) {
				actor_->ReplaceComponent<MeshComponent>(meshComponent);

				if (XMLObjectFile::hasComponent<ShadowSettings>(name_)) {
					Ref<ShadowSettings> SSC = Ref<ShadowSettings>(std::apply([](auto&&... args) {
						RECORD return std::make_shared<ShadowSettings>(args...);
						}, std::tuple_cat(std::make_tuple(actor_.get()), XMLObjectFile::getComponent<ShadowSettings>(name_))));

					if (!actor_->GetComponent<ShadowSettings>()) {
						actor_->AddComponent(SSC);
					}
				}
			}
		}
	}
	// make sure that every actor with material gets a tiling setting defaulted to on
	if (actor_->GetComponent<MeshComponent>() && !actor_->GetComponent<ShadowSettings>())
		actor_->AddComponent<ShadowSettings>(actor_.get(), true);

	if (XMLObjectFile::hasComponent<ScriptComponent>(name_)) {
		std::string scriptName = XMLObjectFile::getComponent<ScriptComponent>(name_);
		for (int i = 1; !scriptName.empty(); i++) {

			RECORD actor_->AddComponent<ScriptComponent>(actor_.get(), AssetManager::getInstance().GetAsset<ScriptAbstract>(scriptName), XMLObjectFile::getPublicVars(name_, i-1));
			scriptName = XMLObjectFile::getComponent<ScriptComponent>(name_, i);

			//in case of infinite error
			if (i == 40) break;
		}
	}
	
	actor_->AddComponent<TransformComponent>(Ref<TransformComponent>(std::apply([](auto&&... args) {
		RECORD return std::make_shared<TransformComponent>(args...);
		}, std::tuple_cat(std::make_tuple(actor_.get()), XMLObjectFile::getComponent<TransformComponent>(name_)))));
	
	if (XMLObjectFile::hasComponent<CameraComponent>(name_)) {
		auto camArgs = XMLObjectFile::getComponent<CameraComponent>(name_);

		// manually settting the arguments in order to pass the actors pointer to the camera
		RECORD Ref<CameraComponent> CamC = std::make_shared<CameraComponent>(
			actor_.get(),
			std::get<1>(camArgs), 
			std::get<2>(camArgs),
			std::get<3>(camArgs),
			std::get<4>(camArgs),
			std::get<5>(camArgs) 
		);

		if (!actor_->GetComponent<CameraComponent>()) {
			actor_->AddComponent(CamC);
		}
	}

	if (XMLObjectFile::hasComponent<LightComponent>(name_)) {
		std::tuple arguments = XMLObjectFile::getComponent<LightComponent>(name_);

		//tried to apply it directly to addcomponent but it always defaulted yet this works fine ¯\_()_/¯			
		Ref<LightComponent> lightG = Ref<LightComponent>(std::apply([](auto&&... args) {
			RECORD return std::make_shared<LightComponent>(args...);
			}, XMLObjectFile::getComponent<LightComponent>(name_)));

		if (!actor_->GetComponent<LightComponent>()) {
			actor_->AddComponent(lightG);
			LightingSystem::getInstance().AddActor(actor_);
		}

		XMLObjectFile::applyLightShadowSettings(name_, actor_->GetComponent<LightComponent>());
	}

	if (XMLObjectFile::hasComponent<AnimatorComponent>(name_)) {
		std::cout << "Has an Animator" << std::endl;

		auto test = std::tuple_cat(std::make_tuple(actor_.get()), XMLObjectFile::getComponent<AnimatorComponent>(name_));

		Ref<AnimatorComponent> animC = Ref<AnimatorComponent>(std::apply([](auto&&... args) {
			RECORD return std::make_shared<AnimatorComponent>(args...);
			}, std::tuple_cat(std::make_tuple(actor_.get()), XMLObjectFile::getComponent<AnimatorComponent>(name_))));

		if (!actor_->GetComponent<AnimatorComponent>()) {

			actor_->AddComponent(animC);

		}


	}

	if (XMLObjectFile::hasComponent<PhysicsComponent>(name_)) {		
		Ref<PhysicsComponent> PC = Ref<PhysicsComponent>(std::apply([](auto&&... args) {
			RECORD return std::make_shared<PhysicsComponent>(args...);
			}, XMLObjectFile::getComponent<PhysicsComponent>(name_)));

		if (!actor_->GetComponent<PhysicsComponent>()) {
			actor_->AddComponent(PC);
			PhysicsSystem::getInstance().AddActor(actor_);
		}
	}

	if (XMLObjectFile::hasComponent<CollisionComponent>(name_)) {		
		Ref<CollisionComponent> CC = Ref<CollisionComponent>(std::apply([](auto&&... args) {
			RECORD return std::make_shared<CollisionComponent>(args...);
			}, XMLObjectFile::getComponent<CollisionComponent>(name_)));

		if (!actor_->GetComponent<CollisionComponent>()) {
			actor_->AddComponent(CC);
			CollisionSystem::getInstance().AddActor(actor_);
		}
	}

	std::string savedTag = XMLObjectFile::readActorTag(name_);
	actor_->setTag(savedTag);

	actor_->OnCreate();
	AddActor(actor_);

	if (actor_->getTag() == "MainCamera" && actor_->GetComponent<CameraComponent>()) {
		SetMainCamera(actor_);
	}
}

std::string SceneGraph::GenerateUniqueActorName(const std::string& originalName) const
{
	std::string baseName = originalName;
	int counter = 1;

	// this whole thing just makes it so that if you are duplicating an actor like Cube_3D, it wont do Cube_3D_1D and it'll make sure its Cube_4D

	// find the start of the duplicated name
	size_t start = originalName.find_last_of('_');

	// make sure that its not the end of the string
	if (start != std::string::npos) {

		// find the end of the duplicated name
		size_t end = originalName.find_last_of('D');

		// get whats inbetween
		if (end == originalName.length() - 1) {
			baseName = originalName.substr(0, start);
		}
	}

	std::string newName;
	do {
		newName = baseName + "_" + std::to_string(counter++) + "D"; // can't end or start with a special character, XML breaks so no ( )
	} while (GetActor(newName) != nullptr);

	return newName;
}

Ref<Actor> SceneGraph::DeepCopyActor(const std::string& newName_, Ref<Actor> original_, Actor* newParent)
{
	auto copy = std::make_shared<Actor>(newParent, newName_);
	copy->setTag(original_->getTag());

	if (auto tc = original_->GetComponent<TransformComponent>()) {
		copy->AddComponent<TransformComponent>(std::make_shared<TransformComponent>(copy.get(), 
			tc->GetPosition(), 
			tc->GetOrientation(), 
			tc->GetScale()));
	}

	if (auto c = original_->GetComponent<MeshComponent>()) copy->ReplaceComponent<MeshComponent>(c);
	if (auto c = original_->GetComponent<MaterialComponent>()) copy->ReplaceComponent<MaterialComponent>(c);
	if (auto c = original_->GetComponent<ShaderComponent>()) copy->ReplaceComponent<ShaderComponent>(c);
	if (auto c = original_->GetComponent<ShadowSettings>()) {
		copy->AddComponent<ShadowSettings>(std::make_shared<ShadowSettings>(copy.get(), c->getCastShadow()));
	}
	if (auto ts = original_->GetComponent<TilingSettings>()) {
		copy->AddComponent<TilingSettings>(std::make_shared<TilingSettings>(copy.get(), ts->getIsTiled(), ts->getTileScale(), ts->getTileOffset()));
	}

	if (auto lc = original_->GetComponent<LightComponent>()) {
		auto newLc = std::make_shared<LightComponent>(copy.get(),
			lc->getType(), lc->getSpec(), lc->getDiff(), lc->getIntensity());
		newLc->setShadowType(lc->getShadowType());
		newLc->setShadowNear(lc->getShadowNear());
		newLc->setShadowFar(lc->getShadowFar());
		newLc->setShadowResolution(lc->getShadowResolution());
		newLc->setShadowOrthoSize(lc->getShadowOrthoSize());
		copy->AddComponent(newLc);
		LightingSystem::getInstance().AddActor(copy);
	}

	if (auto cam = original_->GetComponent<CameraComponent>()) {
		auto newCam = std::make_shared<CameraComponent>(copy.get(),
			cam->getType(), cam->getFOV(),
			cam->getNearClipPlane(), cam->getFarClipPlane(),
			cam->getOrthoSize());
		copy->AddComponent(newCam);
	}

	if (auto pc = original_->GetComponent<PhysicsComponent>()) {
		auto newPc = std::make_shared<PhysicsComponent>(copy.get(),
			pc->getState(), pc->getConstraints(),
			pc->getMass(), pc->getUseGravity(),
			pc->getDrag(), pc->getAngularDrag(),
			pc->getFriction(), pc->getRestitution());
		copy->AddComponent(newPc);
		PhysicsSystem::getInstance().AddActor(copy);
	}

	if (auto cc = original_->GetComponent<CollisionComponent>()) {
		auto newCc = std::make_shared<CollisionComponent>(copy.get(),
			cc->getState(), cc->getType(), cc->getIsTrigger(),
			cc->getRadius(), cc->getCentre(),
			cc->getCentrePosA(), cc->getCentrePosB(),
			cc->getHalfExtents(), cc->getOrientation());
		copy->AddComponent(newCc);
		CollisionSystem::getInstance().AddActor(copy);
	}

	if (auto ac = original_->GetComponent<AnimatorComponent>()) {
		copy->AddComponent<AnimatorComponent>(std::make_shared<AnimatorComponent>(copy.get(),
			ac->getStartTime(), ac->getSpeedMult(), 
			ac->getLoopingState(), ac->getAnimName()));
	}

	for (auto& sc : original_->GetAllComponent<ScriptComponent>()) {
		copy->AddComponent<ScriptComponent>(std::make_shared<ScriptComponent>(copy.get(), sc->getBaseAsset()));
	}

	copy->OnCreate();
	AddActor(copy);

	for (auto& [id, child] : getAllActors()) {
		if (child->getParentActor() == original_.get()) {
			std::string childNewName = GenerateUniqueActorName(child->getActorName());
			DeepCopyActor(childNewName, child, copy.get());
		}
	}

	return copy;
}

Ref<Actor> SceneGraph::InstantiatePrefab(const fs::path& prefabPath, Vec3 position, Quaternion rotation, Actor* parent)
{
	std::vector<Ref<Actor>> allActors;
	Ref<Actor> root = XMLObjectFile::ReadPrefab(prefabPath, allActors);
	if (!root || allActors.empty()) return nullptr;

	if (auto tc = root->GetComponent<TransformComponent>()) tc->SetTransform(position, rotation, tc->GetScale());
	if (parent) root->setParentActor(parent);

	for (auto& actor : allActors) {
		std::string uniqueName = GenerateUniqueActorName(actor->getActorName());
		actor->setActorName(uniqueName);
		AddActor(actor);
	}

	for (auto& actor : allActors) {
		for (auto& sc : actor->GetAllComponent<ScriptComponent>()) {
			Preload(sc.get());
		}
	}

	return root;
}

Ref<Actor> SceneGraph::GetActorCStr(const char* actorName) const {
	return GetActor(actorName);
}

Ref<Actor> SceneGraph::GetActor(const std::string& actorName) const
{
	// try to find the actor by name
	auto nameIt = ActorNameToId.find(actorName);
	if (nameIt == ActorNameToId.end()) {
#ifdef _DEBUG
		Debug::Error("Can't find requested actor: " + actorName, __FILE__, __LINE__);
#endif
		return nullptr;
	}

	// try to find actor by ID
	uint32_t actorId = nameIt->second;
	auto actorIt = Actors.find(actorId);

	if (actorIt != Actors.end()) {
		return actorIt->second;
	}

	// if the actor can't be found by name or by ID
#ifdef _DEBUG

	Debug::Error("Can't find requested actor: " + actorName, __FILE__, __LINE__);
#endif
	return nullptr;
}

std::vector<std::string> SceneGraph::GetAllActorNames() const
{
	std::vector<std::string> allActorNames;
	allActorNames.reserve(Actors.size());

	for (const auto& pair : Actors) {
		allActorNames.push_back(pair.second->getActorName());
	}

	return allActorNames;
}

bool SceneGraph::RemoveActor(const std::string& actorName)
{
	auto nameIt = ActorNameToId.find(actorName);
	if (nameIt == ActorNameToId.end()) {
		Debug::Warning("Actor: " + actorName + " does not exist!", __FILE__, __LINE__);
		return false;
	}

	uint32_t actorId = nameIt->second;
	auto actorIt = Actors.find(actorId);

	if (actorIt == Actors.end()) {
#ifdef _DEBUG

		Debug::Error("Actor: " + actorName + " ID does not exist!", __FILE__, __LINE__);
#endif
		return false;
	}

	Ref<Actor> actorToRemove = actorIt->second;

	// check to see if actor is in lighting system and remove it
	if (actorToRemove->GetComponent<LightComponent>()) {
		LightingSystem::getInstance().RemoveActor(actorToRemove);
	}
	// check to see if actor is in physicssystem and remove it
	if (actorToRemove->GetComponent<PhysicsComponent>()) {
		PhysicsSystem::getInstance().RemoveActor(actorToRemove);
	}
	// check to see if actor is in collisison system and remove it
	if (actorToRemove->GetComponent<CollisionComponent>()) {
		CollisionSystem::getInstance().RemoveActor(actorToRemove);
	}

	actorToRemove->DeleteComponent<CameraComponent>();

	// if the actor that is being removed is parented or a parent, get all children
	std::vector<std::string> childrenToRemove;
	for (const auto& pair : Actors) {
		if (pair.second->getParentActor() == actorToRemove.get()) {
			childrenToRemove.push_back(pair.second->getActorName());
		}
	}

	// recursivly remove each child actor
	for (const std::string& childName : childrenToRemove) {
		RemoveActor(childName);
	}

	// also remove the actor from the debug
	debugSelectedAssets.erase(actorId);

	actorToRemove->OnDestroy();

	if (actorToRemove == m_mainCamera) {
		m_mainCamera = nullptr;
	}

	// remove from both maps
	Actors.erase(actorId);
	ActorNameToId.erase(actorName);

	EditorManager::getInstance().UpdateActorHierarchy();

	return true;
}

void SceneGraph::RemoveAllActors()
{
	// call the OnDestroy for each actor 
	for (auto& pair : Actors) {
		if (pair.second) {
			pair.second->OnDestroy();
		}
	}

	// clear the maps
	m_mainCamera = nullptr;
	Actors.clear();
	ActorNameToId.clear();
	debugSelectedAssets.clear();
	
#ifdef ENGINE_EDITOR
	if (EditorManager::getInstance().IsInitialized()) {
		EditorManager::getInstance().UpdateActorHierarchy();
	}
#endif
}

void SceneGraph::Update(const float deltaTime)
{
	ScriptService::updateAllScripts(deltaTime);

	for (auto& pair : Actors) {
		// get the second value from the pair (actor)
		Ref<Actor> actor = pair.second;
		
		// call physics system update
		if (actor->GetComponent<PhysicsComponent>()) {
			// TODO: fix this, best place to call all functions that need update for engine,
			// between play and edit modes.
		}

		// check for collision system
	}
}

void SceneGraph::Preload(ScriptComponent* script){
	ScriptService::preloadScript(script);
}

Ref<Actor> SceneGraph::GetMainCamera() const
{
	// if main camera already exisits
	if (m_mainCamera && m_mainCamera->GetComponent<CameraComponent>()) {
		return m_mainCamera;
	}

	// a fallback tag search
	for (auto& [id, actor] : Actors) {
		if (actor->getTag() == "MainCamera" && actor->GetComponent<CameraComponent>()) {
			return actor;
		}
	}

	return nullptr;
}

void SceneGraph::SetMainCamera(Ref<Actor> actor_)
{
	if (actor_ && actor_->GetComponent<CameraComponent>()) {
		m_mainCamera = actor_;
	}
}

Ref<Actor> SceneGraph::GetCameraByName(const std::string& name_) const
{
	Ref<Actor> actor = GetActor(name_);
	if (actor && actor->GetComponent<CameraComponent>()) {
		return actor;
	}

	return nullptr;
}
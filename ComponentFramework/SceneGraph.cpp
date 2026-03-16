#include "pch.h"
#include "SceneGraph.h"
#include "XMLManager.h"
#include "InputManager.h"
#include "AnimatorComponent.h"
#include "Skeleton.h"
#include <iostream>
#include <tuple>
#include <string>
#include "HierarchyWindow.h"
#include "EditorManager.h"
#include "PhysicsSystem.h"
#include "CollisionSystem.h"
#include "ColliderDebug.h"
#include "ScreenManager.h"
#include "FBOManager.h"
#include "LightingSystem.h"

SceneGraph::SceneGraph()
{
	pickerShader->OnCreate();

	ScriptService::loadLibraries();
	startMeshLoadingWorkerThread();
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
	//end the mesh loading thread
	stopMeshLoadingWorker();
	
	// TODO: instead of just calling stop straight up to stop scripts, stop scripts directly
	Stop();
	
	RemoveAllActors();
	pickerShader->OnDestroy();
}

void SceneGraph::pushMeshToWorker(Ref<MeshComponent> mesh) {

	if (!mesh->queryLoadStatus()) {
		auto it = std::find(workerQueue.begin(), workerQueue.end(), mesh);
		if (it == workerQueue.end()) {
			workerQueue.push_back(mesh);
		}
	}
}

void SceneGraph::pushAnimationToWorker(Ref<Animation> animation) {

	if (!animation->queryLoadStatus()) {
		auto it = std::find(workerQueue.begin(), workerQueue.end(), animation);
		if (it == workerQueue.end()) {
			workerQueue.push_back(animation);
		}
	}
}

void SceneGraph::stopMeshLoadingWorker()
{
	shouldStop = true;
	if (workerThread.joinable()) {
		workerThread.join();
	}
}
void SceneGraph::meshLoadingWorker()
{
	while (!shouldStop) {

		Ref<MeshComponent> model = nullptr;
		Ref<Animation> animation = nullptr;

		{
			std::lock_guard<std::mutex> lock(queueMutex);
			if (workerQueue.empty()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
				continue;
			}


			model = std::dynamic_pointer_cast<MeshComponent>(workerQueue.back());
			animation = std::dynamic_pointer_cast<Animation>(workerQueue.back());
			workerQueue.pop_back();

		}

		if (model) {

			//std::cout << "Loading Model: " << model->getMeshName() << std::endl;

			model->InitializeMesh();

			scheduleOnMain([model]() {

				model->storeLoadedModel();
				});
		}
		else if (animation) {
			//std::cout << "Loading Animation: " << animation << std::endl;

			animation->InitializeAnimation();



		}
		else {

		}

	}
}

void SceneGraph::processMainThreadTasks() {
	std::unique_lock<std::mutex> lock(taskMutex);
	while (!mainThreadTasks.empty()) {
		auto task = std::move(mainThreadTasks.front());
		mainThreadTasks.pop();
		lock.unlock();  // Release lock before OpenGL
		task();
		lock.lock();
	}
}

void SceneGraph::scheduleOnMain(std::function<void()> task)
{
	{
		std::lock_guard<std::mutex> lock(taskMutex);
		mainThreadTasks.push(std::move(task));
	}
	taskCV.notify_one();
}

bool SceneGraph::queryMeshLoadStatus(std::string name)
{
	for (auto& obj : Actors) {

		Ref<MeshComponent> queriedMesh = obj.second->GetComponent<MeshComponent>();
		if (queriedMesh && queriedMesh->getMeshName() == name.c_str()) {
			return queriedMesh->queryLoadStatus();
		}

	}


	return false;
}

void SceneGraph::startMeshLoadingWorkerThread()
{
	workerThread = std::thread(&SceneGraph::meshLoadingWorker, this);
	//t.detach();              
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
			}
		}
	}

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
			}
		}
	}

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

void SceneGraph::ListAllActors() const
{
	std::cout << "All actors in the scene: " << std::endl;
	for (const auto& pair : Actors) {
		std::cout << pair.second->getActorName() << " (ID: " << pair.first << ")" << std::endl;
	}
}

void SceneGraph::RemoveAllActors()
{
	std::cout << "Deleting All Actors In The Scene" << std::endl;

	LightingSystem::getInstance().ClearActors();
	PhysicsSystem::getInstance().ClearActors();
	CollisionSystem::getInstance().ClearActors();

	// call the OnDestroy for each actor 
	for (auto& pair : Actors) {
		if (pair.second) {
			pair.second->OnDestroy();
		}
	}

	// clear the maps
	m_mainCamera = nullptr;
	Actors.clear();
	EditorManager::getInstance().UpdateActorHierarchy();

	ActorNameToId.clear();
	debugSelectedAssets.clear();
}

void SceneGraph::Update(const float deltaTime)
{
	AnimationClip::updateClipTimes(deltaTime);
	//Load any models that the worker thread finishes loading through assimp
	//storeInitializedMeshData();

	//ScriptService::callActorScripts(GetActor("Cube"), deltaTime)
	
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
	/*if (!(usedCamera == nullptr)) {
		setUsedCamera();
	}*/
}

Ref<Actor> SceneGraph::pickColour(int mouseX, int mouseY) {
	Ref<Actor> mainCamera = GetMainCamera();
	if (!mainCamera) return nullptr;
	Ref<CameraComponent> cam = mainCamera->GetComponent<CameraComponent>();
	if (!cam) return nullptr;
	
	int w = ScreenManager::getInstance().getRenderWidth();
	int h = ScreenManager::getInstance().getRenderHeight();

	FBOData& pickingFBO = FBOManager::getInstance().getFBO(FBO::ColorPicker);

	//use the picking buffer so it is seperated
	glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO.fbo);
	glViewport(0, 0, pickingFBO.width, pickingFBO.height);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//get the special shader for picking and set its uniforms
	glUseProgram(pickerShader->GetProgram());

	glUniformMatrix4fv(pickerShader->GetUniformID("uProjection"), 1, GL_FALSE, cam->GetProjectionMatrix());
	glUniformMatrix4fv(pickerShader->GetUniformID("uView"), 1, GL_FALSE, cam->GetViewMatrix());

	for (auto& actor : Actors) {

		if (!actor.second->GetComponent<MeshComponent>()) { continue; }//no meshcomponent for actor

		//set matrix with its camera (i forgot to put in camera parametre and spent 5 hours why it was coming back as completely black)
		glUniformMatrix4fv(pickerShader->GetUniformID("uModel"), 1, GL_FALSE, actor.second->GetModelMatrix(cam)); //TODO change to editorcamera

		//encode the id of the actor as rgb
		Vec3 idColor = Actor::encodeID(actor.second->getId());

		//send over the rbg to the shader to use as its rendered colour
		glUniform3fv(pickerShader->GetUniformID("uIDColor"), 1, &idColor.x);

		glBindTexture(GL_TEXTURE_2D, 0);
		actor.second->GetComponent<MeshComponent>()->Render();
	}

	//rgb pixel data
	unsigned char pixel[3];

	glReadBuffer(GL_COLOR_ATTACHMENT0);

	// scaled viewport to get uv conversion for fbo coords
	float imgW, imgH;
	float aspect = ScreenManager::getInstance().getRenderAspectRatio();
	Vec2 windowSize = Vec2(InputManager::getInstance().getMouseMap()->dockingSize.x, InputManager::getInstance().getMouseMap()->dockingSize.y);
	GLint xPos = (GLint)InputManager::getInstance().getMouseMap()->dockingPos.x;
	GLint yPos = (GLint)InputManager::getInstance().getMouseMap()->dockingPos.y;

	if (windowSize.x / aspect <= windowSize.y) {
		imgW = windowSize.x;
		imgH = windowSize.x / aspect;
	}
	else {
		imgH = windowSize.y;
		imgW = windowSize.y * aspect;
	}

	float imgX = xPos + (windowSize.x - imgW) * 0.5f;
	float imgY = yPos + (windowSize.y - imgH) * 0.5f;

	float u = ((float)mouseX - imgX) / imgW;
	float v = ((float)mouseY - imgY) / imgH;
	GLint fboX = (GLint)(u * pickingFBO.width);
	GLint fboY = (GLint)((1.0f - v) * pickingFBO.height);

	//get the mouse click's rgb pixel data
	glReadPixels(fboX, fboY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

	//reverse selected pixel's rgb and decode into an id
	uint32_t pickedID = Actor::decodeID(pixel[0], pixel[1], pixel[2]);

	//unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);

	//check if the clicked pixel's colour id is the same as any of the actors
	for (auto& actor : Actors) {
		if (actor.second->getId() == pickedID) return actor.second;
	}

	return nullptr; // nothing clicked
}

void SceneGraph::Render() const
{
	Ref<Actor> mainCamera = GetMainCamera();
	if (!mainCamera) return;
	Ref<CameraComponent> cam = mainCamera->GetComponent<CameraComponent>();
	if (!cam) return;
	
	int w = ScreenManager::getInstance().getRenderWidth();
	int h = ScreenManager::getInstance().getRenderHeight();

	FBOData& sceneFBO = FBOManager::getInstance().getFBO(FBO::Scene);
	FBOData& pickingFBO = FBOManager::getInstance().getFBO(FBO::ColorPicker);

	//use the picking buffer so it is seperated
	glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO.fbo);
	glViewport(0, 0, pickingFBO.width, pickingFBO.height);

	if (!RENDERMAINSCREEN) {
		// upload shader
		LightingSystem::getInstance().UploadUniforms(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Multi")->GetProgram());
		LightingSystem::getInstance().UploadUniforms(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetProgram());
		
		//use the picking buffer so it is seperated
		glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO.fbo);
		glViewport(0, 0, w, h);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//glDisable(GL_BLEND);

	// go through all actors
	for (const auto& pair : Actors) {

		Ref<Actor> actor = pair.second;
		// getting the shader, mesh, and mat for each indivual actor, using mainly for the if statement to check if the actor has each of these components
		Ref<ShaderComponent> shader = actor->GetComponent<ShaderComponent>();

		Ref<MeshComponent> mesh = actor->GetComponent<MeshComponent>();

		bool isSelected = !debugSelectedAssets.empty() && debugSelectedAssets.find(actor->getId()) != debugSelectedAssets.end();

		bool isAnimating = (mesh && actor->GetComponent<AnimatorComponent>() && mesh->skeleton &&
			actor->GetComponent<AnimatorComponent>()->activeClip.getActiveState());

		//replace shader if it should be using another shader for whatever purpose- such as outline.

		if (isAnimating) {
			if (isSelected) {
				shader = AssetManager::getInstance().GetAsset<ShaderComponent>("S_AnimOutline");

			}
			else {
				shader = AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated");
			}
		}
		else if (isSelected) {
			shader = AssetManager::getInstance().GetAsset<ShaderComponent>("S_Outline");

		}

		if (shader) glUseProgram(shader->GetProgram());
		else continue;

		Ref<MaterialComponent> material = actor->GetComponent<MaterialComponent>();

		if (!shader || !mesh || !material) { continue; }

		glUniformMatrix4fv(shader->GetUniformID("uProjection"), 1, GL_FALSE, cam->GetProjectionMatrix());
		glUniformMatrix4fv(shader->GetUniformID("uView"), 1, GL_FALSE, cam->GetViewMatrix());

		if (shader && mesh && material) {
			//MODELMATRIX
			glEnable(GL_DEPTH_TEST);
			glPolygonMode(GL_FRONT_AND_BACK, drawMode);
			Matrix4 modelMatrix = actor->GetModelMatrix();
			glUniformMatrix4fv(shader->GetUniformID("modelMatrix"), 1, GL_FALSE, modelMatrix);


			//ANIMATION
			if (isAnimating) {
				Ref<AnimatorComponent> animComp = actor->GetComponent<AnimatorComponent>();

				double timeInTicks = animComp->activeClip.getCurrentTimeInFrames();

				std::vector<Matrix4> finalBoneMatrices(mesh->skeleton->bones.size(), Matrix4());
				animComp->activeClip.animation->calculatePose(timeInTicks, mesh->skeleton.get(), finalBoneMatrices);



				GLint loc = shader->GetUniformID("bone_transforms[0]");
				if (loc == -1) {
					printf("ERROR: bone_transforms uniform not found!\n");
				}
				else {

					glUniformMatrix4fv(loc, (GLsizei)finalBoneMatrices.size(), GL_FALSE,
						reinterpret_cast<const float*>(finalBoneMatrices.data()));
				}
			}



			//TEXTURE
			glUniform1i(shader->GetUniformID("diffuseTexture"), 0);
			glUniform1i(shader->GetUniformID("specularTexture"), 1);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, material->getDiffuseID());
			if (material->getSpecularID() != 0) {
				glUniform1i(shader->GetUniformID("hasSpec"), 1);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, material->getSpecularID());
			}
			else {
				glUniform1i(shader->GetUniformID("hasSpec"), 0);
			}

			//RENDER
			mesh->Render(GL_TRIANGLES);
			glBindTexture(GL_TEXTURE_2D, 0);

		}

	}

	// collider debug
	glDisable(GL_DEPTH_TEST);
	glLineWidth(2.0f);

	Matrix4 view = cam->GetViewMatrix();
	Matrix4 projection = cam->GetProjectionMatrix();

	for (const auto& selectedPair : debugSelectedAssets) {
		auto actorIt = Actors.find(selectedPair.first);
		if (actorIt == Actors.end()) continue;

		Ref<Actor> actor = actorIt->second;
		Ref<TransformComponent> TC = actor->GetComponent<TransformComponent>();
		Ref<CollisionComponent> CC = actor->GetComponent<CollisionComponent>();

		if (!TC && !CC) continue;

		ColliderDebug::getInstance().Render(CC, TC, view, projection);
	}

	glLineWidth(1.0f);
	glEnable(GL_DEPTH_TEST);

	//
	if (!RENDERMAINSCREEN) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, w, h);
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
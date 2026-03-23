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
#include "glm/gtc/type_ptr.hpp"
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

void SceneGraph::moveUsedCameraTo(Ref<Actor> actor_)
{
	if (EditorManager::getInstance().GetEditorMode() == EditorMode::Play) return;
	//ModelMatrix is in world space, so instead of getting transform local space get the world space and extract the position 
	Vec3 worldPos = actor_->GetModelMatrix() * Vec4(Vec3(), 1);

	Ref<TransformComponent> transfCam = usedCamera->GetUserActor()->GetComponent<TransformComponent>();
	transfCam->SetPos(worldPos.x, worldPos.y, transfCam->GetPosition().z);
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

void SceneGraph::useDebugCamera()
{
	usedCamera = debugCamera->GetComponent<CameraComponent>();

	checkValidCamera();
}

void SceneGraph::setUsedCamera(Ref<CameraComponent> newCam) {
	if (newCam) {
		usedCamera = newCam;
		newCam->fixCameraToTransform();
		checkValidCamera();

	}
	else if (!newCam) {
		useDebugCamera();
	}
}

void SceneGraph::startMeshLoadingWorkerThread()
{
	workerThread = std::thread(&SceneGraph::meshLoadingWorker, this);
	//t.detach();              
}

Ref<CameraComponent> SceneGraph::getUsedCamera() const
{
	if (!usedCamera || !usedCamera->GetUserActor()) {
		return debugCamera->GetComponent<CameraComponent>();
	}

	return usedCamera;
}

void SceneGraph::checkValidCamera()
{
	if (!usedCamera || !usedCamera->GetUserActor()) {
		std::cout << "usedCamera is invalid" << std::endl;



		std::cout << "try debugCam" << std::endl;

		if (debugCamera && debugCamera->GetComponent<CameraComponent>()) {
			std::cout << "DebugCam is found" << std::endl;
		}
		else {

			std::cout << "DebugCam is invalid" << std::endl;
			RECORD debugCamera = std::make_shared<Actor>(nullptr, "cameraDebugOne");
			RECORD debugCamera->AddComponent<TransformComponent>(debugCamera.get(), Vec3(0.0f, 0.0f, 40.0f), QMath::inverse(Quaternion()));
			debugCamera->OnCreate();

			//doesn't need to be added to the sceneGraph
			//sceneGraph.AddActor(debugCamera);

			RECORD debugCamera->AddComponent<CameraComponent>(debugCamera, 45.0f, (16.0f / 9.0f), 0.5f, 300.0f);
			debugCamera->GetComponent<CameraComponent>()->OnCreate();
			debugCamera->GetComponent<CameraComponent>()->fixCameraToTransform();

			setUsedCamera(debugCamera->GetComponent<CameraComponent>());
		}
		usedCamera = debugCamera->GetComponent<CameraComponent>();


	}
}

void SceneGraph::ValidateAllLights()
{
	if (lightActors.size() != 0) {
		for (std::vector<Ref<Actor>>::iterator it = lightActors.begin(); it != lightActors.end(); ++it) {
			if (!(*it)->ValidateLight()) {
				lightActors.erase(std::remove_if(lightActors.begin(), lightActors.end(), [](const Ref<Actor>& actor) { return !actor->ValidateLight(); }), lightActors.end());
			}
		}
	}
}

bool SceneGraph::AddLight(Ref<Actor> actor)
{
	if (!actor->ValidateLight()) return false;

	lightActors.push_back(actor);


	return true;
}

std::vector<Vec3> SceneGraph::GetLightsPos() const
{
	std::vector<Vec3> lightPositions;

	for (auto& obj : lightActors) {
		//lightPositions.push_back(obj->GetComponent<TransformComponent>()->GetPosition());
		lightPositions.push_back(obj->GetModelMatrix() * Vec4(Vec3(), 1));

	}

	return lightPositions;
}

bool SceneGraph::GetLightExist(Ref<Actor> actor)
{
	if (lightActors.size() == 0) return false;
	auto it = std::find(lightActors.begin(), lightActors.end(), actor);

	if (it != lightActors.end()) {
		// Element found
		return true;
	}

	return false;
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
//		lua.script("for k,v in pairs(_G) do if type(v)=='userdata' then _G[k]=nil end end");
		//lua = std::make_unique<sol::state>();
		
		//ScriptService::loadLibraries();



		Ref<AnimatorComponent> actorAnimator =  actor.second->GetComponent<AnimatorComponent>();
		if (actorAnimator) {
			actorAnimator->activeClip.StopPlaying();
			actorAnimator->activeClip.currentTime = 0.0f;
		}
	}

	

	ScriptService::ClearLuaState();

	// Stop physics engine 
	PhysicsSystem::getInstance().ResetPhysics();


	//for (auto& actor : Actors) {
//		ScriptService::preloadActorScripts(actor.second);
	//}

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
	


	/*Ref<TransformComponent> transfC = Ref<TransformComponent>(std::apply([](auto&&... args) {
		return new TransformComponent(args...);
		}, std::tuple_cat(std::make_tuple(actor_.get()), XMLObjectFile::getComponent<TransformComponent>(name_))));*/

	


	actor_->AddComponent<TransformComponent>(Ref<TransformComponent>(std::apply([](auto&&... args) {
		RECORD return std::make_shared<TransformComponent>(args...);
		}, std::tuple_cat(std::make_tuple(actor_.get()), XMLObjectFile::getComponent<TransformComponent>(name_)))));
	


	if (XMLObjectFile::hasComponent<CameraComponent>(name_)) {
		Ref<CameraComponent> CamC = Ref<CameraComponent>(std::apply([](auto&&... args) {
			RECORD return std::make_shared<CameraComponent>(args...);
			}, XMLObjectFile::getComponent<CameraComponent>(name_)));

		if (!actor_->GetComponent<CameraComponent>()) {
			actor_->AddComponent(CamC);
			actor_->GetComponent<CameraComponent>()->setUserActor(actor_);
		}
	}

	if (XMLObjectFile::hasComponent<LightComponent>(name_)) {
		std::cout << "Has a Light" << std::endl;

		std::tuple arguments = XMLObjectFile::getComponent<LightComponent>(name_);
		

		//tried to apply it directly to addcomponent but it always defaulted yet this works fine ¯\_()_/¯			
		Ref<LightComponent> lightG = Ref<LightComponent>(std::apply([](auto&&... args) {
			RECORD return std::make_shared<LightComponent>(args...);
			}, XMLObjectFile::getComponent<LightComponent>(name_)));

		if (!actor_->GetComponent<LightComponent>()) {

			actor_->AddComponent(lightG);

			//Light will be Added to scenegraph after entire actor is loaded fully to avoid issues (in LoadActor function)
			
		}


	}

	if (XMLObjectFile::hasComponent<ShadowSettings>(name_)) {
		Ref<ShadowSettings> SSC = Ref<ShadowSettings>(std::apply([](auto&&... args) {
			RECORD return std::make_shared<ShadowSettings>(args...);
			}, std::tuple_cat(std::make_tuple(actor_.get()), XMLObjectFile::getComponent<ShadowSettings>(name_))));

		if (!actor_->GetComponent<ShadowSettings>()) {
			actor_->AddComponent(SSC);
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

	// check to see if actor is in physicssystem and remove it
	if (actorToRemove->GetComponent<PhysicsComponent>()) {
		PhysicsSystem::getInstance().RemoveActor(actorToRemove);
	}
	// check to see if actor is in physicssystem and remove it
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

	PhysicsSystem::getInstance().ClearActors();
	CollisionSystem::getInstance().ClearActors();
	lightActors.clear();
	// call the OnDestroy for each actor 
	for (auto& pair : Actors) {
		if (pair.second) {
			pair.second->OnDestroy();
		}
	}

	// clear the maps
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

	getUsedCamera()->fixCameraToTransform();

	//	std::cout << usedCamera << std::endl;


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
	//Width of SDL Window
	int w = ScreenManager::getInstance().getRenderWidth();
	int h = ScreenManager::getInstance().getRenderHeight();

	FBOData& pickingFBO = FBOManager::getInstance().getFBO(FBO::ColorPicker);

	float aspectRatio = static_cast<float>(w) / static_cast<float>(h);

	//size of the imgui window used for docking
	Vec2 windowSize = Vec2(InputManager::getInstance().getMouseMap()->dockingSize.x, InputManager::getInstance().getMouseMap()->dockingSize.y);


	//pos of top left corner of docking window
	GLint xPos = (GLint)InputManager::getInstance().getMouseMap()->dockingPos.x;
	GLint yPos = (GLint)InputManager::getInstance().getMouseMap()->dockingPos.y;

	GLsizei xSize;
	GLsizei ySize;




	// Calculate scaled dimensions based on aspect ratio
	if (windowSize.x / aspectRatio <= windowSize.y)
	{
		xSize = (GLsizei)windowSize.x;
		ySize = (GLsizei)(windowSize.x / aspectRatio);
	}
	else
	{
		ySize = (GLsizei)windowSize.y;
		xSize = (GLsizei)(windowSize.y * aspectRatio);
	}


	//Add to the position to move to where the image is centred (non used space in the window would be counted otherwise)
	ImVec2 imagePos = ImVec2((windowSize.x - xSize) * 0.5f, (windowSize.y - ySize) * 0.5f);
	xPos += (GLint)imagePos.x;
	yPos += (GLint)imagePos.y;

	//proper height 
	GLint glY = h - (yPos + ySize);


	//use the picking buffer so it is seperated
	glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO.fbo);
	glViewport(xPos, glY, xSize, ySize);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	//get the special shader for picking and set its uniforms
	glUseProgram(pickerShader->GetProgram());

	glUniformMatrix4fv(pickerShader->GetUniformID("uProjection"), 1, GL_FALSE, getUsedCamera()->GetProjectionMatrix());
	glUniformMatrix4fv(pickerShader->GetUniformID("uView"), 1, GL_FALSE, getUsedCamera()->GetViewMatrix());

	for (auto& actor : Actors) {

		if (!actor.second->GetComponent<MeshComponent>()) { continue; }//no meshcomponent for actor

		//set matrix with its camera (i forgot to put in camera parametre and spent 5 hours why it was coming back as completely black)
		glUniformMatrix4fv(pickerShader->GetUniformID("uModel"), 1, GL_FALSE, actor.second->GetModelMatrix(getUsedCamera()));

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

	//get the mouse click's rgb pixel data
	glReadPixels(mouseX, h - mouseY, 1, 1,
		GL_RGB, GL_UNSIGNED_BYTE, pixel);

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

void SceneGraph::SetUniformShadowPass(Ref<ShaderComponent> shader, Ref<Actor> actor, bool isAnim) const {

	glUseProgram(shader->GetProgram());


	if (!isAnim) glUniform1i(shader->GetUniformID("shadowCondition"), 1);
	else
	{
		glUniform1i(shader->GetUniformID("shadowCondition"), 2);

		Ref<AnimatorComponent> animComp = actor->GetComponent<AnimatorComponent>();

		double timeInTicks = animComp->activeClip.getCurrentTimeInFrames();

		Ref<MeshComponent> mesh = actor->GetComponent<MeshComponent>();

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


	glUniformMatrix4fv(shader->GetUniformID("modelMatrix"), 1, GL_FALSE, actor->GetModelMatrix());

}

void SceneGraph::ShadowPass() const {
	
	int w = ScreenManager::getInstance().getRenderWidth();
	int h = ScreenManager::getInstance().getRenderHeight();
	float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
	FBOData& sceneFBO = FBOManager::getInstance().getFBO(FBO::Scene);

	FBOData& shadowFBO = FBOManager::getInstance().getFBO(FBO::ShadowMap);

	Ref<ShaderComponent> shader = AssetManager::getInstance().GetAsset<ShaderComponent>("S_Shadow");
	glUseProgram(shader->GetProgram());
	//size of the imgui window used for docking
	Vec2 windowSize = Vec2(InputManager::getInstance().getMouseMap()->dockingSize.x, InputManager::getInstance().getMouseMap()->dockingSize.y);


	//pos of top left corner of docking window
	GLint xPos = (GLint)InputManager::getInstance().getMouseMap()->dockingPos.x;
	GLint yPos = (GLint)InputManager::getInstance().getMouseMap()->dockingPos.y;

	GLsizei xSize;
	GLsizei ySize;




	// Calculate scaled dimensions based on aspect ratio
	if (windowSize.x / aspectRatio <= windowSize.y)
	{
		xSize = (GLsizei)windowSize.x;
		ySize = (GLsizei)(windowSize.x / aspectRatio);
	}
	else
	{
		ySize = (GLsizei)windowSize.y;
		xSize = (GLsizei)(windowSize.y * aspectRatio);
	}


	//Add to the position to move to where the image is centred (non used space in the window would be counted otherwise)
	ImVec2 imagePos = ImVec2((windowSize.x - xSize) * 0.5f, (windowSize.y - ySize) * 0.5f);
	xPos += (GLint)imagePos.x;
	yPos += (GLint)imagePos.y;

	//proper height 
	GLint glY = h - (yPos + ySize);


	//use the picking buffer so it is seperated

	Vec4 clearColour = Vec4(0.5f, 0.5f, 0.1f, 0.0f);

	if (!RENDERMAINSCREEN) {
		glBindFramebuffer(GL_FRAMEBUFFER, FBOManager::getInstance().getFBO(FBO::ShadowCubeMap).fbo);
		glViewport(0, 0, FBOManager::getInstance().getFBO(FBO::ShadowCubeMap).width, FBOManager::getInstance().getFBO(FBO::ShadowCubeMap).height);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); // normal culling, not front
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(3.0f, 6.0f);

		glBindFramebuffer(GL_FRAMEBUFFER, FBOManager::getInstance().getFBO(FBO::ShadowCubeMap1).fbo);
		glViewport(0, 0, FBOManager::getInstance().getFBO(FBO::ShadowCubeMap1).width, FBOManager::getInstance().getFBO(FBO::ShadowCubeMap1).height);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); // normal culling, not front
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(3.0f, 6.0f);

		glBindFramebuffer(GL_FRAMEBUFFER, FBOManager::getInstance().getFBO(FBO::ShadowCubeMap2).fbo);
		glViewport(0, 0, FBOManager::getInstance().getFBO(FBO::ShadowCubeMap2).width, FBOManager::getInstance().getFBO(FBO::ShadowCubeMap2).height);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); // normal culling, not front
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(3.0f, 6.0f);

		glBindFramebuffer(GL_FRAMEBUFFER, FBOManager::getInstance().getFBO(FBO::ShadowCubeMap3).fbo);
		glViewport(0, 0, FBOManager::getInstance().getFBO(FBO::ShadowCubeMap3).width, FBOManager::getInstance().getFBO(FBO::ShadowCubeMap3).height);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); // normal culling, not front
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(3.0f, 6.0f);

		glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO.fbo);
		glViewport(0, 0, shadowFBO.width, shadowFBO.height);

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); // normal culling, not front
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(3.0f, 6.0f);


		for (const auto& pair : Actors) {

		
			Ref<MeshComponent> mesh = pair.second->GetComponent<MeshComponent>();
			
			if (!mesh || !pair.second->GetComponent<ShadowSettings>() || !pair.second->GetComponent<ShadowSettings>()->getCastShadow()) continue;
			



			Ref<Actor> actor = pair.second;

			bool isAnimating = (mesh && actor->GetComponent<AnimatorComponent>() && mesh->skeleton &&
				actor->GetComponent<AnimatorComponent>()->activeClip.getActiveState());
			
			SetUniformShadowPass(shader, actor, isAnimating);


			LightType currentLightType = LightType::Sky;
			int pointLightCount = 0;
			int skyLightCount = 0;
			int totalLightCount = 0;
			for (auto& light : lightActors) {
				if (totalLightCount >= 4) break;

				if (light->GetComponent<LightComponent>()->getType() == LightType::Sky) {


					//redo the uniforms when change of light type
					if (currentLightType != LightType::Sky) {
						shader = AssetManager::getInstance().GetAsset<ShaderComponent>("S_Shadow");
						glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO.fbo);
						glViewport(0, 0, shadowFBO.width, shadowFBO.height);

						SetUniformShadowPass(shader, actor, isAnimating);

					}
					currentLightType = LightType::Sky;

					ShadowInfo SInfo = CalculateLightSpaceMatrix(light, LightType::Sky);
					glUniformMatrix4fv(shader->GetUniformID("lightSpaceMatrix"), 1, GL_FALSE, SInfo.SkyMatrix);


					pair.second->GetComponent<MeshComponent>()->Render(GL_TRIANGLES);
					skyLightCount++;

				}
				else if(light->GetComponent<LightComponent>()->getType() == LightType::Point && pointLightCount < 4) {
					
					//each pointlight gets its own cubemap buffer (you could do it in one but the expensive part isn't the buffer but the content so its easier to do with existing framework)
					FBO CubeMapFBO;

					switch (pointLightCount) {
					case 0:
						CubeMapFBO = FBO::ShadowCubeMap;
						break;
					case 1: 
						CubeMapFBO = FBO::ShadowCubeMap1;
						break;
					case 2:
						CubeMapFBO = FBO::ShadowCubeMap2;
						break;
					case 3: 
						CubeMapFBO = FBO::ShadowCubeMap3;
						break;
					default:
						// shouldn't be called
						//CubeMapFBO = FBO::ShadowCubeMap3;
						break;

					}

					//redo the uniforms when change of light type
						shader = AssetManager::getInstance().GetAsset<ShaderComponent>("S_PointShadow");

						glBindFramebuffer(GL_FRAMEBUFFER, FBOManager::getInstance().getFBO(CubeMapFBO).texture);
						glViewport(0, 0, FBOManager::getInstance().getFBO(CubeMapFBO).width, FBOManager::getInstance().getFBO(CubeMapFBO).height);
						SetUniformShadowPass(shader, actor, isAnimating);
					//	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

					
					currentLightType = LightType::Point;
					
				
					ShadowInfo SInfo = CalculateLightSpaceMatrix(light, LightType::Point);



					Vec3 lightPosition = light->GetModelMatrix() * Vec4(Vec3(0, 0, 0), 1);
					glUniform3fv(shader->GetUniformID("lightPos"), 1, lightPosition);
					glUniform1f(shader->GetUniformID("farPlane"), 200);

					glUniformMatrix4fv(shader->GetUniformID("shadowMatrices[0]"), 6, GL_FALSE, reinterpret_cast<const float*>(SInfo.PointMatrices.data()));

					
					//point lights only need to render the regular data, no special light matrixes needed
					pair.second->GetComponent<MeshComponent>()->Render(GL_TRIANGLES);

					pointLightCount++;
				
				}
				
				totalLightCount++;
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO.fbo);
		glViewport(0, 0, w, h);
		glEnable(GL_DEPTH_TEST);
		//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glCullFace(GL_BACK);
	}


}



ShadowInfo SceneGraph::CalculateLightSpaceMatrix(Ref<Actor> lightActor, LightType type) const {
	Matrix4 lightProjection;
	Ref<TransformComponent> transform = lightActor->GetComponent<TransformComponent>();
	ShadowInfo data;
	if (type == LightType::Sky) {


		Vec3 position = lightActor->GetModelMatrix() * Vec4(Vec3(0,0,0),1);

		Quaternion orientation = transform->GetOrientation();

		//Rotation for the 'sun' position in the sky
		Matrix4 cameraWorldTransform =  MMath::toMatrix4(orientation);

		//Orbit's the view like the sun around the camera (kinda similar to how a skybox works)	
		Matrix4 lightView = MMath::translate(Vec3(0, 0, -100)) * MMath::inverse(cameraWorldTransform) * MMath::translate(-(usedCamera->GetUserActor()->GetModelMatrix() * Vec4(Vec3(0,0,0), 1)));

			
		float sceneSize = 60.0f;
		Matrix4 lightProjection = MMath::orthographic(
			-sceneSize, sceneSize,
			-sceneSize, sceneSize,
			0.0f, 200.0f
		);

		//needs -z for some reason unless im doing this wrong ¯\_()_/¯
		Vec3 lightDirFromMatrix = Vec3(lightView[8], lightView[9], -lightView[10]);

		data.SkyMatrix = lightProjection * lightView;
		data.LightDir = lightDirFromMatrix;
	}
	else {


		Vec3 position = lightActor->GetModelMatrix() * Vec4(Vec3(0, 0, 0), 1);

		//position = Vec3(-position.x, position.y, -position.z);
		//position = Vec3(5, 0, 0);

		Matrix4 faceViews[6] = {
			MMath::lookAt(Vec3(-position.x, position.y, position.z), Vec3(-position.x, position.y, position.z) + Vec3(1,  0,  0), Vec3(0, -1,  0)), // +X
			MMath::lookAt(Vec3(-position.x, position.y, position.z), Vec3(-position.x, position.y, position.z) + Vec3(-1,  0,  0), Vec3(0, -1,  0)), // -X
			MMath::lookAt(Vec3(position.x, position.y, -position.z), Vec3(position.x, position.y, -position.z) + Vec3(0, -1,  0), Vec3(0,  0, -1)), // +Y 
			MMath::lookAt(Vec3(position.x, position.y, -position.z), Vec3(position.x, position.y, -position.z) + Vec3(0,  1,  0), Vec3(0,  0,  1)), // -Y 
			MMath::lookAt(Vec3(position.x, position.y, -position.z), Vec3(position.x, position.y, -position.z) + Vec3(0,  0,  1), Vec3(0, -1,  0)), // +Z
			MMath::lookAt(Vec3(position.x, position.y, -position.z), Vec3(position.x, position.y, -position.z) + Vec3(0,  0, -1), Vec3(0, -1,  0)), // -Z
		};
		lightProjection = MMath::perspective(90.0f, 1.0f, 0.0001f, 200.0f);

		

		data.type = LightType::Point;
		for (int i = 0; i < 6; i++) {
			data.PointMatrices.push_back(lightProjection * (faceViews[i]));
		}
		/*data.PointMatrices.push_back(lightProjection *
			MMath::lookAt(position, position + Vec3(1.0, 0.0, 0.0), Vec3(0.0, -1.0, 0.0)));

		data.PointMatrices.push_back(lightProjection *
			MMath::lookAt(position, position + Vec3(-1.0, 0.0, 0.0), Vec3(0.0, -1.0, 0.0)));
		data.PointMatrices.push_back(lightProjection *
			MMath::lookAt(position, position + Vec3(0.0, -1.0, 0.0), Vec3(0.0, 0.0, 1.0)));
		data.PointMatrices.push_back(lightProjection *
			MMath::lookAt(position, position + Vec3(0.0, 1.0, 0.0), Vec3(0.0, 0.0, -1.0)));
		data.PointMatrices.push_back(lightProjection *
			MMath::lookAt(position, position + Vec3(0.0, 0.0, 1.0), Vec3(0.0, -1.0, 0.0)));
		data.PointMatrices.push_back(lightProjection *
			MMath::lookAt(position, position + Vec3(0.0, 0.0, -1.0), Vec3(0.0, -1.0, 0.0)));*/


		/*for (int i = 0; i < 6; ++i) {
			data.PointMatrices.push_back(lightProjection * MMath::lookAt(position, position + target[i], up[i]));
		}*/
	}
	return data;
}

void SceneGraph::Render() const
{

	int w = ScreenManager::getInstance().getRenderWidth();
	int h = ScreenManager::getInstance().getRenderHeight();

	FBOData& sceneFBO = FBOManager::getInstance().getFBO(FBO::Scene);
	FBOData& pickingFBO = FBOManager::getInstance().getFBO(FBO::ColorPicker);

	float aspectRatio = static_cast<float>(w) / static_cast<float>(h);

	//size of the imgui window used for docking
	Vec2 windowSize = Vec2(InputManager::getInstance().getMouseMap()->dockingSize.x, InputManager::getInstance().getMouseMap()->dockingSize.y);


	//pos of top left corner of docking window
	GLint xPos = (GLint)InputManager::getInstance().getMouseMap()->dockingPos.x;
	GLint yPos = (GLint)InputManager::getInstance().getMouseMap()->dockingPos.y;

	GLsizei xSize;
	GLsizei ySize;




	// Calculate scaled dimensions based on aspect ratio
	if (windowSize.x / aspectRatio <= windowSize.y)
	{
		xSize = (GLsizei)windowSize.x;
		ySize = (GLsizei)(windowSize.x / aspectRatio);
	}
	else
	{
		ySize = (GLsizei)windowSize.y;
		xSize = (GLsizei)(windowSize.y * aspectRatio);
	}


	//Add to the position to move to where the image is centred (non used space in the window would be counted otherwise)
	ImVec2 imagePos = ImVec2((windowSize.x - xSize) * 0.5f, (windowSize.y - ySize) * 0.5f);
	xPos += (GLint)imagePos.x;
	yPos += (GLint)imagePos.y;

	//proper height 
	GLint glY = h - (yPos + ySize);


	//use the picking buffer so it is seperated
	glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO.fbo);
	glViewport(xPos, glY, xSize, ySize);

	if (!RENDERMAINSCREEN) {
		Vec4 clearColour = Vec4(0.1f, 0.1f, 0.15f, 0.0f);


		std::vector<Vec3> lightPos;
		std::vector<Vec4> lightSpec;
		std::vector<Vec4> lightDiff;
		std::vector<float> lightIntensity;
		std::vector<GLuint> lightTypes;
		if (!lightActors.empty()) {
			for (auto& light : lightActors) {
				if (light->GetComponent<LightComponent>()->getType() == LightType::Point) {
					lightPos.push_back(Vec3(light->GetModelMatrix() * Vec4(Vec3(0,0,0), 1))); // "Colunm" scott-typo lol
					lightTypes.push_back(1u);
				}
				else {

					lightPos.push_back(VMath::normalize(light->GetComponent<TransformComponent>()->GetForward()));
					lightTypes.push_back(0u);
				}
				lightSpec.push_back(light->GetComponent<LightComponent>()->getSpec());
				lightDiff.push_back(light->GetComponent<LightComponent>()->getDiff());
				lightIntensity.push_back(light->GetComponent<LightComponent>()->getIntensity());
			}

			glUniform3fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Multi")->GetUniformID("lightPos[0]"), lightActors.size(), lightPos[0]);
			glUniform4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Multi")->GetUniformID("diffuse[0]"), lightActors.size(), lightDiff[0]);
			glUniform4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Multi")->GetUniformID("specular[0]"), lightActors.size(), lightSpec[0]);
			glUniform1fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Multi")->GetUniformID("intensity[0]"), lightActors.size(), lightIntensity.data());
			glUniform1uiv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Multi")->GetUniformID("lightType[0]"), lightActors.size(), lightTypes.data());

			glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetProgram());

			glUniform3fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetUniformID("lightPos[0]"), lightActors.size(), lightPos[0]);
			glUniform4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetUniformID("diffuse[0]"), lightActors.size(), lightDiff[0]);
			glUniform4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetUniformID("specular[0]"), lightActors.size(), lightSpec[0]);
			glUniform1fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetUniformID("intensity[0]"), lightActors.size(), lightIntensity.data());
			glUniform1uiv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetUniformID("lightType[0]"), lightActors.size(), lightTypes.data());

		}
		glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Multi")->GetProgram());

		glUniform1ui(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Multi")->GetUniformID("numLights"), lightActors.size());

		// Without an Ambient the light components won't work at all
		glUniform4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Multi")->GetUniformID("ambient"), 1, clearColour);


		glUseProgram(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetProgram());

		glUniform1ui(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetUniformID("numLights"), lightActors.size());

		glUniform4fv(AssetManager::getInstance().GetAsset<ShaderComponent>("S_Animated")->GetUniformID("ambient"), 1, clearColour);

		//use the picking buffer so it is seperated
		glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO.fbo);
		glViewport(0, 0, w, h);
		glClearColor(clearColour.x, clearColour.y, clearColour.z, clearColour.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	}

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	ShadowPass();

	


	Ref<Actor> skyLight;

	for (auto& li : lightActors) {
		if (li->GetComponent<LightComponent>() && li->GetComponent<LightComponent>()->getType() == LightType::Sky) {
			skyLight = li;
			break;
		}
	}



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

		glUniformMatrix4fv(shader->GetUniformID("uProjection"), 1, GL_FALSE, getUsedCamera()->GetProjectionMatrix());
		glUniformMatrix4fv(shader->GetUniformID("uView"), 1, GL_FALSE, getUsedCamera()->GetViewMatrix());

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

			if (!isSelected) {

				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, FBOManager::getInstance().getFBO(FBO::ShadowMap).texture);
				glUniform1i(shader->GetUniformID("shadowMap"), 2);
				Vec3 camPos = getUsedCamera()->GetUserActorTransform()->GetPosition();
				glUniform3fv(shader->GetUniformID("cameraPos"), 1, camPos);

				int allowedPointLights = 4;

				if (skyLight) {
					allowedPointLights--;
					ShadowInfo lightInfo = CalculateLightSpaceMatrix(skyLight, LightType::Sky);
					glUniformMatrix4fv(shader->GetUniformID("lightSpaceMatrix"), 1, GL_FALSE, lightInfo.SkyMatrix);
					glUniform3fv(shader->GetUniformID("shadowLightDir"), 1, lightInfo.LightDir);
				}



				//ShadowInfo lightInfo = CalculateLightSpaceMatrix(skyLight, LightType::Point);
				//glUniform3fv(shader->GetUniformID("lightPos[0]"), 1, skyLight->GetModelMatrix() * Vec4(Vec3(), 1));

				std::vector<GLuint> shadowCubemaps = { 
					FBOManager::getInstance().getFBO(FBO::ShadowCubeMap).texture, 
					FBOManager::getInstance().getFBO(FBO::ShadowCubeMap1).texture,
					FBOManager::getInstance().getFBO(FBO::ShadowCubeMap2).texture,
					FBOManager::getInstance().getFBO(FBO::ShadowCubeMap3).texture
				};



				for (int i = 0; i < shadowCubemaps.size(); i++) {
					glActiveTexture(GL_TEXTURE3 + i); // GL_TEXTURE3, 4, 5, 6
					glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubemaps[i]);
				}

				// Upload array of sampler indices to shader
				int samplerIndices[4] = { 3, 4, 5, 6 };
				glUniform1iv(shader->GetUniformID("pointShadowMaps[0]"), allowedPointLights, samplerIndices);

			}

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

	Matrix4 view = getUsedCamera()->GetViewMatrix();
	Matrix4 projection = getUsedCamera()->GetProjectionMatrix();

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

	//stop to prevent memory leaks if you close on run
	Stop();
	
	RemoveAllActors();
	pickerShader->OnDestroy();
}


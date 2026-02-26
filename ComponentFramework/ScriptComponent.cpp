#include "pch.h"
#include "ScriptComponent.h"
#include "SceneGraph.h"
#include "ScriptAbstract.h"
#include "InputCreatorManager.h"
#include "InputManager.h"
#include "CollisionSystem.h"
static std::vector<ScriptComponent*> scriptsInUse;

ScriptComponent::ScriptComponent(Component* parent_, Ref<ScriptAbstract> baseScriptAsset, std::vector<float> pubVars_) :
	Component(parent_), baseAsset(baseScriptAsset) {

	if (baseScriptAsset) {
		setFilenameFromAbstract(baseScriptAsset);
	}


	if (!pubVars_.empty()) {
		
		int i = 0;
		for (auto& pair : publicVariables) {

			if (i < pubVars_.size())
				setPublicReference(pair.first, pubVars_[i]);


			i++;
		}
	}
}

ScriptComponent::~ScriptComponent() {

	std::vector<ScriptComponent*>::iterator it = std::find(scriptsInUse.begin(), scriptsInUse.end(), this);

	if (it != scriptsInUse.end()) {
		scriptsInUse.erase(it);
	}
	else {
#ifdef _DEBUG
		std::cerr << "ERROR: " << this << " was not found as a valid used script upon destroy (Do not remove scripts from the system until they are destroyed.)" << std::endl;
#endif
	}
}

void ScriptComponent::OnDestroy() {
	std::vector<ScriptComponent*>::iterator it = std::find(scriptsInUse.begin(), scriptsInUse.end(), this);

	if (it != scriptsInUse.end()) {
		scriptsInUse.erase(it);
	}

}

void ScriptComponent::Update(const float deltaTime) {
	std::cout << "Hello from Update " << deltaTime << '\n';
}

///sets the filename to the name stored by a ScriptAbstract dropped from AssetManager onto a Actor's component in InspectorWindow. (Friend class inspectorwindow()
void ScriptComponent::setFilenameFromAbstract(Ref<ScriptAbstract> baseScript)
{
	baseAsset = baseScript;

	filename = baseAsset->getName();
	load_lua_file();
	std::vector<ScriptComponent*>::iterator it = std::find(scriptsInUse.begin(), scriptsInUse.end(), this);



	if (it != scriptsInUse.end()) {
		//script is in already
	}
	else {
		//script is not considered valid, add to list of valid

		scriptsInUse.push_back(this);
		SceneGraph::getInstance().Preload(this);
	}

}


void ScriptComponent::Render()const {}


void ScriptComponent::setLocal(const std::string& name, sol::object value) {
	persistentLocals[name] = value;
	if (persistentLocals[name].get_type() != sol::type::function) {
		if (name._Starts_with("PUBLIC__")) {

			//if you have public before a variable itll add that variable to a special list that can be edited the script menu
			if (!publicVariables[name.substr(8)].valid())
			publicVariables[name.substr(8)] = value;
		}
	}


}

void ScriptComponent::setPublicReference(const std::string refName, float ref)
{
	if (publicVariables[refName].valid()) {		
		lua.registry().set(refName, ref);                         

		publicVariables[refName] = lua.registry()[refName];
		
		std::cout << publicVariables[refName].as<float>() << std::endl;
	}
}

std::unordered_map<std::string, sol::object> ScriptComponent::getPublicReferences() {
	return publicVariables;
}



sol::object ScriptComponent::getLocal(const std::string& name) {
	auto it = persistentLocals.find(name);
	return it != persistentLocals.end() ? it->second : sol::lua_nil;
}

bool ScriptComponent::hasLocal(const std::string& name) {
	return persistentLocals.find(name) != persistentLocals.end();
}


void ScriptComponent::load_lua_file() {


	std::ifstream file(getPath(), std::ios::in | std::ios::binary);

	
	if (!file.is_open()) {
#ifdef _DEBUG
		std::cerr << "[ERROR] Failed to open Lua file: " << getPath() << std::endl;
#endif
		return;
	}

	
	std::stringstream buffer;
	buffer << file.rdbuf(); // read the entire file into the stringstream
	code = buffer.str();    // return as a std::string



	

	std::string from("public ");
	std::string to("PUBLIC__");


	if (!from.empty()) {
		size_t start_pos = 0;
		while ((start_pos = code.find(from, start_pos)) != std::string::npos) {
			code.replace(start_pos, from.length(), to);
			// Advance the starting position past the new 'to' string to avoid infinite loops
			start_pos += to.length();
		}
	}

	
}

///Use as the function to compile, isCreated will turn false if the function script has a compilation error.
bool ScriptComponent::OnCreate()
{	
	load_lua_file();

	
	SceneGraph::getInstance().Preload(this);

	/*sol::error err = loaded_script;
	std::cerr << "Lua compile error: " << err.what() << std::endl;*/


	/*try {
		lua.script(R"(
            print("Hello from LuaJIT!")
            function add(a, b)
                return a + b
            end
        )");
	}
	catch (const sol::error& e) {
		std::cerr << "Lua error: " << e.what() << std::endl;
	}*/

	//int result = lua["add"](10, 32);
	
	return true;
}

void ScriptService::startActorScripts(Ref<Actor> target) {

	for (auto& comp : target->components) {



		//filter out non scripts, but don't break as we want to check for multiple scripts on an actor
		if (!std::dynamic_pointer_cast<ScriptComponent>(comp)) continue;

		Ref<ScriptComponent> script = std::dynamic_pointer_cast<ScriptComponent>(comp);

		//lets preload an extra time in case you changed the script. Should be fine as the first preload was mainly for preloading animations or meshes.
		//ScriptService::preloadScript(script.get());

	//	ScriptService::preloadScript(script.get());

		//No point in continuing if its already started
		Actor* user = dynamic_cast<Actor*>(script->parent);

		if (script->isCreated == true) continue;

		
		

		sol::load_result loaded_script = lua.load(script->code);

		//Check runtime vs compiler errors, (Not writing anything just a check)
		if (!loaded_script.valid()) {
			sol::error err = loaded_script;
#ifdef _DEBUG
			std::cerr << "[ERROR] Lua compile error: " << err.what() << std::endl;
#endif
			script->isCreated = false;
			continue;
		}
		else if (!user) {
#ifdef _DEBUG
			std::cerr << "[ERROR] Lua compile error: " << script.get() << " has no user!" << std::endl;
#endif
			script->isCreated = false;
			continue;
		}
		else {
			try {

				lua["Script"] = script;

				//restore the variables previously set

				//define AFTER so that we don't accidentally give an unusable reference
				defineUsertypes(user);

				lua.script(R"(
    setmetatable(_G, {
    __index = function(t, k) 
        return Script:GetLocal(k) or rawget(t, k)
    end,
    __newindex = function(t, k, v) 
        Script:SetLocal(k, v)  -- Breakpoint FIRES!
    end
})
)");
				loaded_script();


				script->restorePublicVars();

				script->restoreAll();


				lua["Start"]();
				script->isCreated = true;



			}
			catch (const sol::error& e) {
#ifdef _DEBUG
				std::cerr << "[ERROR] Lua runtime error: " << e.what() << std::endl;
#endif
				//Runtime error, so disable script
				script->isCreated = false;
				//Disable play mode 
			}
		}

	}
}

void ScriptService::stopActorScripts(Ref<Actor> target)
{
	for (auto& comp : target->components) {

		//filter out non scripts, but don't break as we want to check for multiple scripts on an actor
		if (!std::dynamic_pointer_cast<ScriptComponent>(comp)) continue;

		Ref<ScriptComponent> script = std::dynamic_pointer_cast<ScriptComponent>(comp);

		//No point in continuing if its already started
		script->isCreated = false;


		//Safety check if script crashes it sometimes wouldn't remove itself on stop from valid script use vector
		std::vector<ScriptComponent*>::iterator it = std::find(scriptsInUse.begin(), scriptsInUse.end(), script.get());

		if (it != scriptsInUse.end()) {
			scriptsInUse.erase(it);
		}
		
	}
}

void ScriptService::preloadActorScripts(Ref<Actor> target) {
	for (auto& comp : target->components) {
		if (!std::dynamic_pointer_cast<ScriptComponent>(comp)) continue;
		Ref<ScriptComponent> script = std::dynamic_pointer_cast<ScriptComponent>(comp);

		ScriptService::preloadScript(script.get());

	}
}

void ScriptService::updateAllScripts(float deltaTime) {
	for (auto& script : scriptsInUse) {


		Actor* user = dynamic_cast<Actor*>(script->parent);

		if (user == nullptr) { continue; }

		if (script->isCreated == false) continue;

		if (script->filename.empty()) continue;


		sol::load_result loaded_script = lua.load(script->code);


		
			//Script should be good to run
			try {
				lua["Script"] = script;

				//restore the variables previously set

				//define AFTER so that we don't accidentally give an unusable reference
				defineUsertypes(user);

				lua.script(R"(
    setmetatable(_G, {
    __index = function(t, k) 
        return Script:GetLocal(k) or rawget(t, k)
    end,
    __newindex = function(t, k, v) 
        Script:SetLocal(k, v)  -- Breakpoint FIRES!
    end
})
)");


				//loaded_script();

				script->restoreAll();


				sol::protected_function update = lua["Update"];
				if (update.valid()) {
					auto result = update(deltaTime);
					if (!result.valid()) {
						sol::error err = result;
#ifdef _DEBUG
						std::cerr << "[ERROR] Update failed: " << err.what() << std::endl;
#endif
						script->isCreated = false;
						continue;
					}
				}

			}
			catch (const sol::error& e) {
#ifdef _DEBUG
				std::cerr << "[ERROR] Lua runtime error: " << e.what() << std::endl;
#endif
				//Runtime error, so disable script
				script->isCreated = false;

				//Disable play mode 
			}
			catch (...) {
#ifdef _DEBUG
				std::cerr << "[ERROR] Unknown Lua panic - state corrupted!" << std::endl;
#endif
				script->isCreated = false;
				// Consider recreating lua state here for safety
			}
	}

	
}

void ScriptService::defineUsertypes(Actor* user) {

	
	/*lua.script(R"(
    setmetatable(_G, {
    __index = function(t, k) 
        return Script:GetLocal(k) or rawget(t, k)
    end,
    __newindex = function(t, k, v) 
    end
})
)");*/

	lua["Transform"] = sol::lua_nil;  // Reset first or it'll not be consistent 
	lua["Transform"] = user->GetComponent<TransformComponent>();


	lua["GameObject"] = sol::lua_nil;  // Reset first or it'll not be consistent 
	lua["GameObject"] = user;

	lua["Game"] = sol::lua_nil;
	lua["Game"] = &SceneGraph::getInstance();
	
}



void ScriptService::preloadScript(ScriptComponent* script) {

	std::cout << std::endl;
		Actor* user = dynamic_cast<Actor*>(script->parent);

		if (user == nullptr) { return; }

		if (script->filename.empty()) return;

		

		sol::load_result loaded_script = lua.load(script->code);


		if (!loaded_script.valid()) {

			sol::error err = loaded_script;
#ifdef _DEBUG
			std::cerr << "[ERROR] Lua compile error: " << err.what() << std::endl;
#endif
			//Disable script by setting isCreated to false
			script->isCreated = false;
			return;
		}
		else {

			//Script should be good to run
			try {
				lua["Script"] = script;
				
				defineUsertypes(user);

				lua.script(R"(
    setmetatable(_G, {
    __index = function(t, k) 
        return Script:GetLocal(k) or rawget(t, k)
    end,
    __newindex = function(t, k, v) 
        Script:SetLocal(k, v)  -- Breakpoint FIRES!
    end
})
)");

				
	
				//load the original script. Don't use for start and update as that will reset all the stored variables
				loaded_script();



				sol::protected_function preload = lua["Preload"];
				if (preload.valid()) {
					auto result = preload();
					if (!result.valid()) {
						sol::error err = result;
#ifdef _DEBUG
						std::cerr << "[ERROR] Preload failed: " << err.what() << std::endl;
#endif
						script->isCreated = false;
						return;
					}
				}

			}
			catch (const sol::error& e) {
#ifdef _DEBUG
				std::cerr << "[ERROR] Lua runtime error: " << e.what() << std::endl;
#endif
				//Runtime error, so disable script
				script->isCreated = false;

				//Disable play mode 
			}
			catch (...) {
#ifdef _DEBUG
				std::cerr << "[ERROR] Unknown Lua panic - state corrupted!" << std::endl;
#endif
				script->isCreated = false;
				// Consider recreating lua state here for safety
			}
		}
		
		if (!script->pubVars.empty()) {
			int i = 0;
			for (auto& pair : script->publicVariables) {

				auto& obj = pair.second;

				script->setPublicReference(pair.first, script->pubVars[i]);




				i++;
			}
		}

		script->restorePublicVars();

	
}



bool ScriptService::libLoaded = false;

void ScriptService::loadLibraries()
{
	if (libLoaded) return;

	lua.open_libraries(
		sol::lib::base,    // print, assert, etc.
		sol::lib::math,    // math functions
		sol::lib::table,   // table manipulation
		sol::lib::string,  // string functions
		sol::lib::os       // os functions
	);

	//Establishes TransformComponent as Transform in lua
	/*lua.new_usertype<TransformComponent>("Transform",
		"x", sol::property(&TransformComponent::GetX, &TransformComponent::SetX),
		"y", sol::property(&TransformComponent::GetY, &TransformComponent::SetY),
		"z", sol::property(&TransformComponent::GetZ, &TransformComponent::SetZ)
	);*/


	void(TransformComponent::*TRANSFORM_SETPOSVEC3)(Vec3) = &TransformComponent::SetPos;
	void(TransformComponent::*TRANSFORM_SETPOSFLOAT)(float,float,float) = &TransformComponent::SetPos;


	lua.new_usertype<TransformComponent>("Transform",
		//Write new functions to include parent's transform and get global transforms
		"Position", sol::property(&TransformComponent::GetPosition, TRANSFORM_SETPOSVEC3),
		"Rotation", sol::property(&TransformComponent::GetOrientation, &TransformComponent::SetOrientation),
		"Scale", sol::property(&TransformComponent::GetScale, &TransformComponent::SetScale),
		"GameObject", sol::property(&TransformComponent::getParent)

	);

	lua.new_usertype<ScriptComponent>("ScriptComponent",
		"SetLocal", &ScriptComponent::setLocal,
		"GetLocal", &ScriptComponent::getLocal,
		"HasLocal", &ScriptComponent::hasLocal
	);

	lua.new_usertype<PhysicsComponent>("Rigidbody",
		"Vel", sol::property(&PhysicsComponent::getVel, &PhysicsComponent::setVel),
		"AddAccel", &PhysicsComponent::AddForce,
		"Drag", sol::property(&PhysicsComponent::getDrag, &PhysicsComponent::setDrag),
		"AngVel", sol::property(&PhysicsComponent::getAngularVel, &PhysicsComponent::setAngularVel),
		"AddAngAccel", &PhysicsComponent::AddTorque,
		"AngDrag", sol::property(&PhysicsComponent::getAngularDrag, &PhysicsComponent::setAngularDrag),
		"UseGravity", sol::property(&PhysicsComponent::getUseGravity, &PhysicsComponent::setUseGravity),
		"Mass", sol::property(&PhysicsComponent::getMass, &PhysicsComponent::setMass),
		"State", sol::property(&PhysicsComponent::getState, &PhysicsComponent::setState)
	);

	lua.new_usertype<RaycastHit>("RaycastHit", 
		"IsHit", sol::property([](const RaycastHit& h) {return h.isIntersected; }),
		"GameObject", sol::property([](const RaycastHit& h) {return h.hitActor; }),
		"Distance", sol::property([](const RaycastHit& h) {return h.t; }),
		"Normal", sol::property([](const RaycastHit& h) {return h.normal; }),
		"Point", sol::property([](const RaycastHit& h) {return h.intersectionPoint; })
		);

	lua.new_usertype<CameraComponent>("Camera",
		"FOV", sol::property(&CameraComponent::getFOV, &CameraComponent::setFOV),
		"NearClipPlane", sol::property(&CameraComponent::getNearClipPlane, &CameraComponent::setNearClipPlane),
		"FarClipPlane", sol::property(&CameraComponent::getFarClipPlane, &CameraComponent::setFarClipPlane),
		"AspectRatio", sol::property(&CameraComponent::getAspectRatio, &CameraComponent::setAspectRatio),
		"Transform", sol::property(&CameraComponent::GetUserActorTransform)
	);

	lua.new_usertype<AnimationClip>("AnimationClip",
		sol::constructors<AnimationClip()>(),


		"GetPlayState", &AnimationClip::isPlaying,
		"Length", sol::property(& AnimationClip::getClipLength),
		"GetLength", &AnimationClip::getClipLength,
		"Loop", sol::property(&AnimationClip::getLoopingState, &AnimationClip::setLooping),
		"CurrentTime", sol::property(&AnimationClip::getCurrentTime, &AnimationClip::setCurrentTime),
		"StartTime", sol::property(&AnimationClip::getStartTime, &AnimationClip::setStartTime),
		"SpeedMult", sol::property(&AnimationClip::getSpeedMult, &AnimationClip::setSpeedMult),
		"SetAnimation", &AnimationClip::setAnimationStr,
		"GetAnimationName", &AnimationClip::getAnimNameCStr,
		"GetAnimationFilename", &AnimationClip::getAnimFilename,
		"GetState", &AnimationClip::getActiveState,
		"PreloadAnimation", &AnimationClip::preloadAnimation
	);

	lua.new_usertype<AnimatorComponent>("Animator",
		"Play", &AnimatorComponent::PlayClip,
		"Stop", &AnimatorComponent::StopClip,
		"GetPlayState", &AnimatorComponent::isPlaying,
		"Length", sol::property(&AnimatorComponent::getClipLength),
		"GetLength", &AnimatorComponent::getClipLength,
		"Loop", sol::property(&AnimatorComponent::getLoopingState, &AnimatorComponent::setLooping),
		"CurrentTime", sol::property(&AnimatorComponent::getCurrentTime, &AnimatorComponent::setCurrentTime),
		"StartTime", sol::property(&AnimatorComponent::getStartTime, &AnimatorComponent::setStartTime),
		"SpeedMult", sol::property(&AnimatorComponent::getSpeedMult, &AnimatorComponent::setSpeedMult),
		"Clip", sol::property(&AnimatorComponent::getAnimationClip, &AnimatorComponent::setAnimationClip)
	);



	lua.new_usertype<Actor>("GameObject",
		"Transform", sol::property(&Actor::GetComponent<TransformComponent>),
		"Camera", sol::property(&Actor::GetComponent<CameraComponent>),
		"Animator", sol::property(&Actor::GetComponent<AnimatorComponent>),
		"Rigidbody", sol::property(& Actor::GetComponent<PhysicsComponent>),
		"Name", sol::property(&Actor::getActorName),
		"Tag", sol::property(&Actor::getTag, &Actor::setTag),
		"CompareTag", &Actor::compareTag
	);

	// Input Manager Lua
	
	// table with input states
	lua["InputState"] = lua.create_table_with(
		"Released", static_cast<int>(InputState::Released),
		"Pressed", static_cast<int>(InputState::Pressed),
		"Held", static_cast<int>(InputState::Held)
	);

	// for getting the key
	sol::table tab = lua.create_table();
	tab.set_function("GetInputState", &InputCreatorManager::getInputState);

	// helper functions for getting if key down, pressed, released
	tab.set_function("IsKeyDown", [](const std::string& key) -> bool {
		int state = InputCreatorManager::getInputState(key);
		return state == static_cast<int>(InputState::Pressed) || state == static_cast<int>(InputState::Held);
		});

	tab.set_function("IsKeyPressed", [](const std::string& key) -> bool {
		int state = InputCreatorManager::getInputState(key);
		return state == static_cast<int>(InputState::Pressed); 
		});
	
	tab.set_function("IsKeyReleased", [](const std::string& key) -> bool {
		int state = InputCreatorManager::getInputState(key);
		return state == static_cast<int>(InputState::Released); 
		});

	// Mouse functions
	// getting the inputmanager mouse map directly instead of creating something for the inputcreatormanager,
	// there is only like 3 mouse states we need to get anyways instead of the like 100s on a keyboard
	tab.set_function("GetMouseButton", [](int button) -> int {
		InputManager& im = InputManager::getInstance();
		return static_cast<int>(im.getMouseMap()->getInputState(button));
		});

	// helper functions for getting if key down, pressed, released
	tab.set_function("IsMouseButtonDown", [](int button) -> bool {
		InputManager& im = InputManager::getInstance();
		InputState s = im.getMouseMap()->getInputState(button);
		return s == InputState::Pressed || s == InputState::Held;
		});

	tab.set_function("IsMouseButtonPressed", [](int button) -> bool {
		InputManager& im = InputManager::getInstance();
		InputState s = im.getMouseMap()->getInputState(button);
		return s == InputState::Pressed;
		});

	tab.set_function("IsMouseButtonReleased", [](int button) -> bool {
		InputManager& im = InputManager::getInstance();
		InputState s = im.getMouseMap()->getInputState(button);
		return s == InputState::Released;
		});

	lua.new_usertype<SceneGraph>("Game",
		"Find", &SceneGraph::GetActorCStr, //I'd make this a lambda but const char* needs the function to be const which can't be done to lambdas
		"UsedCamera", sol::property([&]() { return SceneGraph::getInstance().getUsedCamera(); }),
		"Input", tab
	);
	

	//put overload parametres in the second brackets of the LHS
	const Vec3(Vec3::*VEC3_UNARY_MINUS)() const = &Vec3::operator-;
	const Vec3(Vec3::*VEC3_SUBTRACT)(const Vec3&) const = &Vec3::operator-;

	const Vec3(Vec3::*VEC3_MULTIPLY_FLOAT)(const float) const = &Vec3::operator*;


	//friend functions has to be lambda unfortunately due to not being a member of Vec3
	const auto FLOAT_MULTIPLY_VEC3 = [](const float s, const Vec3& v) {
		return v * s;
	};


	const Quaternion(Quaternion::*QUATERNION_UNARY_MINUS)() const = &Quaternion::operator-;
	const Quaternion(Quaternion::*QUATERNION_SUBTRACT)(const Quaternion) const = &Quaternion::operator-;
	const Quaternion(Quaternion::* QUATERNION_MULTIPLY_FLOAT)(const float) const = &Quaternion::operator*;
	const Vec3(Quaternion::* QUATERNION_MULTIPLY_VEC3)(const Vec3&) const = &Quaternion::operator*;
	//Quaternion h makes this function a friend function which causes issues with this due to changing the function from being a member of quaternion to a member of nothing (non member)
	//Since we can't use the non-member (friend) function, lets just use a lambda.
	const auto VEC3_MULTIPLY_QUATERNION = [](const Vec3 v, const Quaternion& q) {
		Quaternion qv(0.0f, v);
		Quaternion result = qv * q;
		return result.ijk;
		};



	//const Vec3 v, const Quaternion& q
	// 
	//Vec3 Def

	

	



	lua.new_usertype<Vec3>("Vec3",
		sol::constructors<Vec3(), Vec3(float, float, float)>(),

		//values
		"x", &Vec3::x,
		"y", &Vec3::y,
		"z", &Vec3::z,

		//print
		sol::meta_function::to_string, [](const Vec3& v) { //Vec3 can't be made into a string because its a custom structure so manually gotta fix it
			return "Vec3(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
		},

		//operators
		sol::meta_function::addition, &Vec3::operator+,
		sol::meta_function::unary_minus, VEC3_UNARY_MINUS,
		sol::meta_function::subtraction, VEC3_SUBTRACT, //lua does not know which meta function to use when doing Vec3:operator-, so do a lambda
		sol::meta_function::multiplication, sol::overload(VEC3_MULTIPLY_FLOAT, FLOAT_MULTIPLY_VEC3, VEC3_MULTIPLY_QUATERNION),
		sol::meta_function::division, &Vec3::operator/

	);


	
	//Quaternion Def
	lua.new_usertype<Quaternion>("Quaternion",
		sol::constructors<Quaternion(), Quaternion(float, Vec3)>(),

		//values
		"w", &Quaternion::w,
		"ijk", &Quaternion::ijk,

		//print
		sol::meta_function::to_string, [](const Quaternion& q) {
			return "Quaternion(" + std::to_string(q.w) + ", " + std::to_string(q.ijk.x) + ", " + std::to_string(q.ijk.y) + ", " + std::to_string(q.ijk.z) + ")";
		},

		//operators
		sol::meta_function::addition, &Quaternion::operator+,
		sol::meta_function::unary_minus, QUATERNION_UNARY_MINUS,
		sol::meta_function::subtraction, QUATERNION_SUBTRACT, 
		sol::meta_function::multiplication, sol::overload(QUATERNION_MULTIPLY_VEC3, QUATERNION_MULTIPLY_FLOAT),
		sol::meta_function::division, & Quaternion::operator/	

	);


	
	
	

	


	lua["QMath"] = sol::new_table();
	{
		lua["QMath"]["Conjugate"].set_function(&QMath::conjugate);
		lua["QMath"]["AngleAxisRotation"].set_function(&QMath::angleAxisRotation);
		lua["QMath"]["Dot"].set_function(&QMath::dot);
		lua["QMath"]["Inverse"].set_function(&QMath::inverse);
		lua["QMath"]["LookAt"].set_function(&QMath::lookAt);
		lua["QMath"]["Magnitude"].set_function(&QMath::magnitude);
		lua["QMath"]["Normalize"].set_function(&QMath::normalize);
		lua["QMath"]["Pow"].set_function(&QMath::pow);
		lua["QMath"]["Rotate"].set_function(&QMath::rotate);
		lua["QMath"]["Slerp"].set_function(&QMath::slerp);
	}

	//I didn't add implimentation for Vec2 or Vec4, I only did Vec3
	lua["VMath"] = sol::new_table();
	{

		lua["VMath"]["Lerp"].set_function(&VMath::lerp);
		lua["VMath"]["Distance"].set_function(&VMath::distance);
		lua["VMath"]["Reflect"].set_function(&VMath::reflect);
		lua["VMath"]["Rotate"].set_function(&VMath::rotate);


		//same as quat and vec3, except that because its static, remove the namespace in (Vec3::*VEC3_Cross)
		const Vec3(*VEC3_CROSS)(const Vec3&, const Vec3&) = &VMath::cross;
		lua["VMath"]["Cross"].set_function(VEC3_CROSS);

		float(*VEC3_DOT)(const Vec3&, const Vec3&) = &VMath::dot;
		lua["VMath"]["Dot"].set_function(VEC3_DOT);

		float(*VEC3_MAG)(const Vec3&) = &VMath::mag;
		lua["VMath"]["Magnitude"].set_function(VEC3_MAG);

		Vec3(*VEC3_NORMALIZE)(const Vec3&) = &VMath::normalize;
		lua["VMath"]["Normalize"].set_function(VEC3_NORMALIZE);

	}

	// new physics table, for now just putting raycast functions in here
	lua["Physics"] = sol::new_table();
	{
		// similar to unity https://docs.unity3d.com/6000.0/Documentation/ScriptReference/RaycastHit.html

		// manually setting this function so that distance can have an optional default for distance
		lua["Physics"]["Raycast"].set_function([](const Vec3& origin, const Vec3& direction, sol::optional<float> maxDistance) {
			return CollisionSystem::getInstance().Raycast(origin, direction, maxDistance.value_or(FLT_MAX));
			});

		lua["Physics"]["RaycastAll"].set_function([](const Vec3& origin, const Vec3& direction, sol::optional<float> maxDistance) {
			std::vector<RaycastHit> hits = CollisionSystem::getInstance().RaycastAll(origin, direction, maxDistance.value_or(FLT_MAX));
			
			// returning a table of all hits
			sol::table result = lua.create_table();
			for (size_t i = 0; i < hits.size(); i++) {
				result[static_cast<int>(i) + 1] = hits[i];
			}
			return result;
			});

		// screenraycast function, gets sdl mouse coords 
		lua["Physics"]["ScreenRaycast"].set_function([]() {
			int x, y;
			SDL_GetMouseState(&x, &y);
			return CollisionSystem::getInstance().ScreenRaycast(x, y);
			});
	}

	

}

void ScriptService::callScriptCollision(Ref<ScriptComponent> script, Ref<Actor> other, CollisionDetectionState type) {


		//No point in continuing if its already started
		Actor* user = dynamic_cast<Actor*>(script->parent);

		if (script->isCreated == false) return;


			try {

				lua["Script"] = script;

				//restore the variables previously set

				//define AFTER so that we don't accidentally give an unusable reference
				defineUsertypes(user);

				lua.script(R"(
    setmetatable(_G, {
    __index = function(t, k) 
        return Script:GetLocal(k) or rawget(t, k)
    end,
    __newindex = function(t, k, v) 
        Script:SetLocal(k, v)  -- Breakpoint FIRES!
    end
})
)");


				script->restoreAll();


				switch (type) {

				case CollisionDetectionState::Enter:
					lua["OnCollisionEnter"](other);

					break;
				case CollisionDetectionState::Stay:
					lua["OnCollisionStay"](other);

					break;
				case CollisionDetectionState::Exit:
					lua["OnCollisionExit"](other);

					break;
				case CollisionDetectionState::TriggerEnter:
					lua["OnTriggerEnter"](other);

					break;
				case CollisionDetectionState::TriggerStay:
					lua["OnTriggerStay"](other);

					break;
				case CollisionDetectionState::TriggerExit:
					lua["OnTriggerExit"](other);

					break;
				default:

					break;
				}


			}
			catch (const sol::error& e) {
#ifdef _DEBUG
				std::cerr << "[ERROR] Lua runtime error: " << e.what() << std::endl;
#endif
				//Runtime error, so disable script
				script->isCreated = false;
				//Disable play mode 
			}
		

	
}


void ScriptService::registerCollisionDetection(Ref<Actor> actor1_, Ref<Actor> actor2_, CollisionDetectionState type)
{
	if (actor1_->GetComponent<ScriptComponent>()) {
		for (auto& script : actor1_->GetAllComponent<ScriptComponent>()) {
			callScriptCollision(script, actor2_, type);
		}
	}
	if (actor2_->GetComponent<ScriptComponent>()) {
		for (auto& script : actor2_->GetAllComponent<ScriptComponent>()) {
			callScriptCollision(script, actor1_, type);
		}
	}
}
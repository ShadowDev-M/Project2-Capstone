#include "ScriptComponent.h"
#include <filesystem>
#include <fstream>
#include "SceneGraph.h"

static std::vector<ScriptComponent*> scriptsInUse;

ScriptComponent::ScriptComponent(Component* parent_, const char* filename_) :
	Component(parent_), filename(filename_) {
	scriptsInUse.push_back(this);
}

ScriptComponent::~ScriptComponent() {

	std::vector<ScriptComponent*>::iterator it = std::find(scriptsInUse.begin(), scriptsInUse.end(), this);

	if (it != scriptsInUse.end()) {
		scriptsInUse.erase(it);
	}
	else {
		std::cerr << "ERROR: " << this << " was not found as a valid used script upon destroy (Do not remove scripts from the system until they are destroyed.)" << std::endl;
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


void ScriptComponent::Render()const {}

void ScriptComponent::load_lua_file() {
	std::ifstream file(getPath(), std::ios::in | std::ios::binary);

	
	if (!file.is_open()) {
		std::cerr << "[ERROR] Failed to open Lua file: " << getPath() << std::endl;
		return;
	}

	
	std::stringstream buffer;
	buffer << file.rdbuf(); // read the entire file into the stringstream
	code = buffer.str();    // return as a std::string
}

///Use as the function to compile, isCreated will turn false if the function script has a compilation error.
bool ScriptComponent::OnCreate()
{	
	load_lua_file();


	

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

		//No point in continuing if its already started
		if (script->isCreated == true) continue;

		
		sol::load_result loaded_script = lua.load(script->code);

		//Check runtime vs compiler errors, (Not writing anything just a check)
		if (!loaded_script.valid()) {
			sol::error err = loaded_script;
			std::cerr << "[ERROR] Lua compile error: " << err.what() << std::endl;
			script->isCreated = false;
			continue;
		}
		else {
			try {


				loaded_script();
				lua["Transform"] = target->GetComponent<TransformComponent>();
				lua["Start"]();
				script->isCreated = true;


			}
			catch (const sol::error& e) {
				std::cerr << "[ERROR] Lua runtime error: " << e.what() << std::endl;
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

		

		
	}
}

void ScriptService::updateAllScripts(float deltaTime) {
	for (auto& script : scriptsInUse) {


		Actor* user = dynamic_cast<Actor*>(script->parent);

		if (user == nullptr) { continue; }

		if (script->isCreated == false) continue;

		if (script->filename.empty()) continue;

		sol::load_result loaded_script = lua.load(script->code);


		if (!loaded_script.valid()) {

			sol::error err = loaded_script;
			std::cerr << "[ERROR] Lua compile error: " << err.what() << std::endl;

			//Disable script by setting isCreated to false
			script->isCreated = false;
			continue;
		}
		else {

			//Script should be good to run
			try {


				loaded_script();

				lua["Transform"] = sol::lua_nil;  // Reset first or it'll not be consistent 
				lua["Transform"] = user->GetComponent<TransformComponent>();
				sol::protected_function update = lua["Update"];
				if (update.valid()) {
					auto result = update(deltaTime);
					if (!result.valid()) {
						sol::error err = result;
						std::cerr << "[ERROR] Update failed: " << err.what() << std::endl;
						script->isCreated = false;
						continue;
					}
				}

			}
			catch (const sol::error& e) {
				std::cerr << "[ERROR] Lua runtime error: " << e.what() << std::endl;
				//Runtime error, so disable script
				script->isCreated = false;

				//Disable play mode 
			}
			catch (...) {
				std::cerr << "[ERROR] Unknown Lua panic - state corrupted!" << std::endl;
				script->isCreated = false;
				// Consider recreating lua state here for safety
			}
		}

	}
}


void ScriptService::callActorScripts(Ref<Actor> target, float deltaTime)
{
	//Verify the script is a component of the target before calling
	for (auto& comp : target->components) {

		if (!std::dynamic_pointer_cast<ScriptComponent>(comp)) continue;

		Ref<ScriptComponent> script = std::dynamic_pointer_cast<ScriptComponent>(comp);


		if (script->isCreated == false) continue;

		sol::load_result loaded_script = lua.load(script->code);

		
		if (!loaded_script.valid()) {

			sol::error err = loaded_script;
			std::cerr << "[ERROR] Lua compile error: " << err.what() << std::endl;

			//Disable script by setting isCreated to false
			script->isCreated = false;
			continue;
		}
		else {

			//Script should be good to run
			try {
				

				loaded_script();

				lua["Transform"] = sol::lua_nil;  // Reset first or it'll not be consistent 
				lua["Transform"] = target->GetComponent<TransformComponent>();
				sol::protected_function update = lua["Update"];
				if (update.valid()) {
					auto result = update(deltaTime);
					if (!result.valid()) {
						sol::error err = result;
						std::cerr << "[ERROR] Update failed: " << err.what() << std::endl;
						script->isCreated = false;
						continue;
					}
				}

			}
			catch (const sol::error& e) {
				std::cerr << "[ERROR] Lua runtime error: " << e.what() << std::endl;
				//Runtime error, so disable script
				script->isCreated = false;

				//Disable play mode 
			}
			catch (...) {
				std::cerr << "[ERROR] Unknown Lua panic - state corrupted!" << std::endl;
				script->isCreated = false;
				// Consider recreating lua state here for safety
			}
		}

	}
	
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

	lua.new_usertype<TransformComponent>("Transform",
		//Write new functions to include parent's transform and get global transforms
		"Position", sol::property(&TransformComponent::GetPosition, &TransformComponent::SetPos),
		"Rotation", sol::property(&TransformComponent::GetQuaternion, &TransformComponent::SetOrientation)


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
		lua["QMath"]["Conjugate"] = &QMath::conjugate;
		lua["QMath"]["AngleAxisRotation"] = &QMath::angleAxisRotation;
		lua["QMath"]["Dot"] = &QMath::dot;
		lua["QMath"]["Inverse"] = &QMath::inverse;
		lua["QMath"]["LookAt"] = &QMath::lookAt;
		lua["QMath"]["Magnitude"] = &QMath::magnitude;
		lua["QMath"]["Normalize"] = &QMath::normalize;
		lua["QMath"]["Pow"] = &QMath::pow;
		lua["QMath"]["Rotate"] = &QMath::rotate;
		lua["QMath"]["Slerp"] = &QMath::slerp;
	}

	//I didn't add implimentation for Vec2 or Vec4, I only did Vec3
	lua["VMath"] = sol::new_table();
	{

		lua["VMath"]["Lerp"] = &VMath::lerp;
		lua["VMath"]["Distance"] = &VMath::distance;
		lua["VMath"]["Reflect"] = &VMath::reflect;
		lua["VMath"]["Rotate"] = &VMath::rotate;


		//same as quat and vec3, except that because its static, remove the namespace in (Vec3::*VEC3_Cross)
		const Vec3(*VEC3_CROSS)(const Vec3&, const Vec3&) = &VMath::cross;
		lua["VMath"]["Cross"] = VEC3_CROSS;

		float(*VEC3_DOT)(const Vec3&, const Vec3&) = &VMath::dot;
		lua["VMath"]["Dot"] = VEC3_DOT;

		float(*VEC3_MAG)(const Vec3&) = &VMath::mag;
		lua["VMath"]["Magnitude"] = VEC3_MAG;

		Vec3(*VEC3_NORMALIZE)(const Vec3&) = &VMath::normalize;
		lua["VMath"]["Normalize"] = VEC3_NORMALIZE;

	}


}

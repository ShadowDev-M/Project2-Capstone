#include "ScriptComponent.h"
#include <filesystem>
#include <fstream>
#include "SceneGraph.h"

ScriptComponent::ScriptComponent(Component* parent_, const char* filename_) :
	Component(parent_), filename(filename_) {}

ScriptComponent::~ScriptComponent() {}

void ScriptComponent::OnDestroy() {}

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
				lua["Transform"] = target->GetComponent<TransformComponent>();
				lua["Update"](deltaTime);


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
		sol::meta_function::unary_minus, [](const Vec3& v) { return -v; },
		sol::meta_function::subtraction, [](const Vec3& a, const Vec3& b) { return a - b; }, //lua does not know which meta function to use when doing Vec3:operator-, so do a lambda
		sol::meta_function::multiplication, &Vec3::operator*,
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
		sol::meta_function::addition, & Quaternion::operator+,
		sol::meta_function::unary_minus, [](const Quaternion& q) { return -q; },
		sol::meta_function::subtraction, [](const Quaternion& a, const Quaternion& b) { return a - b; }, 
		//sol::meta_function::multiplication, [](const Quaternion& a, const Quaternion& b) { return a * b; },
		sol::meta_function::division, & Quaternion::operator/

	);



}

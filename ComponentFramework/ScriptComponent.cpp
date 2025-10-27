#include "ScriptComponent.h"
#include <filesystem>
#include <fstream>


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

bool ScriptComponent::OnCreate()
{	
	lua.open_libraries(
		sol::lib::base,    // print, assert, etc.
		sol::lib::math,    // math functions
		sol::lib::table,   // table manipulation
		sol::lib::string,  // string functions
		sol::lib::os       // os functions
	);

	//swap this with 
	/*std::string code = R"(
        print("Hello from LuaJIT!")
        function add(a, b)
            return a + b
        end
    )";*/

	load_lua_file();

	sol::load_result loaded_script = lua.load(code);


	//Check runtime vs compiler errors
	if (!loaded_script.valid()) {
		sol::error err = loaded_script;
		std::cerr << "[ERROR] Lua compile error: " << err.what() << std::endl;
	}
	else {
		try {
			loaded_script(); 

			int result = lua["add"](1, 1);
			std::cout << result << std::endl;
		}
		catch (const sol::error& e) {
			std::cerr << "[ERROR] Lua runtime error: " << e.what() << std::endl;
		}
	}

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

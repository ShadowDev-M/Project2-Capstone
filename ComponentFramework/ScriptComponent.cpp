#include "ScriptComponent.h"
#include <cstdlib> 
#include <filesystem>

//Path for script files
const char* ScriptManager::PATH = "";


std::unordered_map<std::string, std::ofstream> ScriptManager::scriptList;

void ScriptManager::Open_File(const std::string& name)
{

    //check if script should be allowed to open
    if (true) {
        
        //find the ofstream for the pushed script, and then open the file via path
        //scriptList.find(name)->second.open(PATH + name);



    }
    else {
        std::cerr << "Failed to open file " << name << "!\n";
    }
}

void ScriptManager::addScriptIncludeToVcxproj(const std::string& scriptPath, const std::string& vcxprojPath) {
    std::ifstream inFile(vcxprojPath);
    if (!inFile.is_open()) {
        std::cerr << "Cannot open project file!\n";
        return;
    }

    std::string content((std::istreambuf_iterator<char>(inFile)),
        std::istreambuf_iterator<char>());
    inFile.close();

    // Check if the file is already included
    if (content.find(scriptPath) != std::string::npos) {
        std::cout << "File already included in project.\n";
        return;
    }

    // Determine file type
    std::string itemTag;
    if (scriptPath.size() >= 2 && scriptPath.substr(scriptPath.size() - 2) == ".h")
        itemTag = "<ClInclude Include=\"" + scriptPath + "\" />";
    else if (scriptPath.size() >= 4 && scriptPath.substr(scriptPath.size() - 4) == ".cpp")
        itemTag = "<ClCompile Include=\"" + scriptPath + "\" />";
    else {
        std::cerr << "Unsupported file type.\n";
        return;
    }

    // Insert the line before closing </Project> tag
    size_t pos = content.find("</Project>");
    if (pos != std::string::npos) {
        std::string insertText = "  <ItemGroup>\n    " + itemTag + "\n  </ItemGroup>\n";
        content.insert(pos, insertText);

        std::ofstream outFile(vcxprojPath);
        outFile << content;
        outFile.close();

        std::cout << "Added " << scriptPath << " to project.\n";
    }
    else {
        std::cerr << "Invalid project file.\n";
    }
}

void ScriptManager::OpenFileForUser(const std::string& filename) {
    // Create the file if it doesn't exist



    //check if windows, if not- why would you be using anything else with visual studio
#if defined(_WIN32)
    std::string command = "start \"\" \"" + Get_Path(filename) + "\"";
#else
    std::cerr << "Unsupported OS" << std::endl;
    return;
#endif

    // Open the file in the default text editor
    system(command.c_str());
}

std::string ScriptManager::Get_Path(const std::string& name)
{
    return PATH + name + ".cpp";
}

bool ScriptManager::Push_Script(const std::string& name)
{
    bool isNew = true;
    if (std::filesystem::exists(Get_Path(name))) {
        isNew = false;
        scriptList.insert(
            std::make_pair(name,
                //call appended to not overwrite
                std::ofstream(Get_Path(name), std::ios::app)
            )
        );
    }
    else {
        scriptList.insert(
            std::make_pair(name,
                std::ofstream(Get_Path(name))
            )
        );
    }

    

    std::ofstream* txtFile = &scriptList.find(name)->second;

    //Logic for writing basic setup for script
    if (isNew) {
        *txtFile << "#pragma once" << std::endl;
        *txtFile << std::endl;

        *txtFile << "#include \"ScriptComponent.h\"" << std::endl;
        *txtFile << "#include <iostream>" << std::endl;



        *txtFile << std::endl;
        *txtFile << std::endl;

        *txtFile << "class " << name << ": public Script {" << std::endl; // write text

        *txtFile << std::endl;
        *txtFile << "public:" << std::endl;
        *txtFile << std::endl;
        *txtFile << "   " << name << "() {" << std::endl;
        *txtFile << "       ScriptManager::scriptsRuntimeVector.push_back(this); " << std::endl;
        *txtFile << "   }" << std::endl;

        *txtFile << "   void Start() {" << std::endl;
        *txtFile << "       std::cout << \"ive run!\" << std::endl;" << std::endl;

        *txtFile << std::endl;
        *txtFile << "   }" << std::endl;
        *txtFile << "   void Update(float deltaTime) {" << std::endl;
        *txtFile << std::endl;
        *txtFile << "   }" << std::endl;
        *txtFile << "};" << std::endl;

        *txtFile << std::endl;
        *txtFile << "static " << name << " instance;" << std::endl;

        
    }





    //close after pushing
    scriptList.find(name)->second.close();

    if (isNew) {


        addScriptIncludeToVcxproj(Get_Path(name), "ComponentFramework.vcxproj");


        std::filesystem::path msbuildPath = R"(C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe)"; 
        std::string command = "\"" + msbuildPath.string() + "\" ComponentFramework.vcxproj /t:Rebuild /p:Configuration=Debug";



        system(command.c_str());
    }

    

    return true;
}

std::vector<Script*> ScriptManager::scriptsRuntimeVector;
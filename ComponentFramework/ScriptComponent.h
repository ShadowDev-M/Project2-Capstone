#pragma once
#include <fstream>
#include <iostream> 
#include <vector>
#include <unordered_map>
    //std::ofstream outputFile("example.txt"); // Creates or opens "example.txt"

    //if (outputFile.is_open()) { // Check if the file was opened successfully
    //    outputFile << "This is some text for the new file." << std::endl;
    //    outputFile << "Another line of content." << std::endl;
    //    outputFile.close();
    //}

///base class for each script
class Script {

public:

    virtual void Start() {}


    virtual void Update(float deltaTime) {}

};



class ScriptManager {

    //defined in cpp file
    static const char* PATH;
 
    //list of scripts in use
    static std::unordered_map<std::string, std::ofstream> scriptList;
    
    
    static void addScriptIncludeToVcxproj(const std::string& scriptPath, const std::string& vcxprojPath);


public:

    static void BuildSingleCpp(const std::filesystem::path& msbuildExe,
        const std::filesystem::path& projectFile,
        const std::filesystem::path& cppFile);

    static std::vector<Script*> scriptsRuntimeVector;

    static void OpenFileForUser(const std::string& filename);

    static std::string Get_Path(const std::string& = "");

    static bool Push_Script(const std::string& name);

    //static bool ValidScript(std::string name);

    static void Open_File(const std::string& name);

   // static bool Is_Open_File(std::string name);



};


#pragma once
#include "Actor.h"

//lua handler
static sol::state lua;

//should be called before any lua scripts are added

class ScriptAbstract;

class ScriptComponent : public Component {
	//Make SceneGraph friend class to allow SceneGraph to authorize/deauthorize an actor's usage of a script by adding/removing it to users
	//Also you could remove it if you wanted to disable it 
	friend class ScriptService;
	friend class InspectorWindow;
	friend class XMLObjectFile;

	ScriptComponent(const ScriptComponent&) = delete;
	ScriptComponent(ScriptComponent&&) = delete;
	ScriptComponent& operator = (const ScriptComponent&) = delete;
	ScriptComponent& operator = (ScriptComponent&&) = delete;

	const std::string SCRIPTSPATH = "scripts/";
	std::string filename;
	
	std::string code;

	//Asset and Component names are different sometimes so just point to the base asset (Important for save files as you need the asset and can't use GetAsset(name) because the name may be different)
	Ref<ScriptAbstract> baseAsset;

	//Dangerous to let any actor trigger a script, much better to have a manifest controlled by assetmanager/scenegraph for safety
	std::vector<Ref<Actor>> users;

	void load_lua_file();

	//Only SceneGraph/AssetManager should call update (friend class)
	void Update(const float deltaTime_);

	void setFilenameFromAbstract(Ref<ScriptAbstract> baseScript);

public:
	Ref<ScriptAbstract> getBaseAsset() { return baseAsset; }
	ScriptComponent(Component* parent, Ref<ScriptAbstract> baseScriptAsset = 0);
	virtual ~ScriptComponent();
	
	const std::string getPath() { return SCRIPTSPATH + filename; }

	std::string getName() { return filename; }

	bool OnCreate();
	void OnDestroy();
	void Render() const;

};


class ScriptService {
	//Scene graph is the only one who should be using ScriptService
	static bool libLoaded;

	friend class SceneGraph;
private:
	static void callActorScripts(Ref<Actor> target, float deltaTime);
	static void startActorScripts(Ref<Actor> target);
	static void stopActorScripts(Ref<Actor> target);

	static void updateAllScripts(float deltaTime);

public:

	static void loadLibraries();


};



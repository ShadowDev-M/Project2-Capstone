#pragma once

#include "TransformComponent.h"  
#include "CameraComponent.h"  
#include "LightComponent.h"  
#include "ScriptComponent.h"
#include "CollisionComponent.h"
#include "TilingSettings.h"
#include "SceneGraph.h"
#include "ShadowSettings.h"

template <typename T>
constexpr auto TypeName = static_cast<std::string>(typeid(T).name()).substr(6);


using namespace tinyxml2;

class XMLObjectFile {
    /// <summary>
    /// recursivly adds the actor's from a cell file into the scene
    /// </summary>
    /// <param name="sceneGraph">scene you are loading actors to </param>
    /// <param name="attribute">pointer to the current element you are adding </param>
    static void createActorFromElement(SceneGraph* sceneGraph, XMLElement* actorElement);

    static XMLElement* writeActorRecursive(Actor* actor_, XMLElement* root_);

    static void runCreateActorsOfElementChildren(SceneGraph* sceneGraph, XMLElement* actorElement, XMLElement* rootElement = nullptr);

    static fs::path BuildPath(const std::string& relative) {
        fs::path path = SearchPath::getInstance().GetRoot() / relative;
        fs::create_directories(path.parent_path());
        return path;
    }

    // prefab helpers
    static void WritePrefabNode(XMLDocument& doc, XMLElement* parentEl,
        const std::string& actorName);
    static Ref<Actor> ReadPrefabNode(XMLElement* nodeEl, Actor* parent);

public:
    // manifest file creation
    // these manifests are basically just xmls but with different extension names
    static void WriteMatManifest(const fs::path& outputPath, const std::string& diffuseRelative, const std::string& specularRelative = "", const std::string& normalRelative = "");
    static void WriteShaderManifest(const fs::path& outputPath, const std::string& vertRel, const std::string& fragRel, const std::string& tessCtrlRel = "", const std::string& tessEvalRel = "", const std::string& geomRel = "");

    // prefab read and write
    static bool WritePrefab(const std::string& actorName, const fs::path& outputPath);
    static Ref<Actor> ReadPrefab(const fs::path& prefabPath);

    /// Creates an object file for the actor
    static int writeActor(std::string name) {

        XMLDocument doc;

        XMLNode* cRoot = doc.NewElement("Actor");
        doc.InsertFirstChild(cRoot);
        
        std::string path = BuildPath("Game Objects/" + SceneGraph::getInstance().sceneFileName + "/" + name + ".xml").string();

        doc.SaveFile(path.c_str());
        return 0;
    }

    static int writeSceneFile(std::string name) {

        XMLDocument doc;

        XMLNode* cRoot = doc.NewElement("Scene");
        doc.InsertFirstChild(cRoot);

        std::string path = BuildPath("Scenes/" + name + ".scene").string();
        doc.SaveFile(path.c_str());
        return 0;
    }

    static int writeSceneTags(const std::string& sceneName, const std::vector<std::string>& tags)
    {
        std::string path = BuildPath("Scenes/" + sceneName + ".scene").string();
        XMLDocument doc;

        XMLError eResult = doc.LoadFile(path.c_str());
        if (eResult != XML_SUCCESS) return eResult;

        XMLNode* cRoot = doc.RootElement();

        XMLElement* existing = cRoot->FirstChildElement("Tags");
        if (existing) cRoot->DeleteChild(existing);

        XMLElement* tagsElement = doc.NewElement("Tags");
        for (const std::string& tag : tags) {
            XMLElement* tagElement = doc.NewElement("Tag");
            tagElement->SetAttribute("value", tag.c_str());
            tagsElement->InsertEndChild(tagElement);
        }
        cRoot->InsertFirstChild(tagsElement);

        return doc.SaveFile(path.c_str());
    }

    static std::vector<std::string> readSceneTags(const std::string& sceneName)
    {
        std::string path = BuildPath("Scenes/" + sceneName + ".scene").string();
        
        std::vector<std::string> tags;

        XMLDocument doc;
        if (doc.LoadFile(path.c_str()) != XML_SUCCESS) return tags;

        XMLNode* cRoot = doc.RootElement();
        if (!cRoot) return tags;

        XMLElement* tagsElement = cRoot->FirstChildElement("Tags");
        if (!tagsElement) return tags;

        for (XMLElement* tag = tagsElement->FirstChildElement("Tag"); tag != nullptr; tag = tag->NextSiblingElement("Tag")) {
            const char* val = tag->Attribute("value");
            if (val) tags.push_back(val);
        }

        return tags;
    }

    static int writeActorTag(const std::string& actorName, const std::string& tag)
    {
        std::string path = BuildPath("Game Objects/" + SceneGraph::getInstance().sceneFileName + "/" + actorName + ".xml").string();

        XMLDocument doc;

        XMLError eResult = doc.LoadFile(path.c_str());
        if (eResult != XML_SUCCESS) return eResult;

        XMLNode* cRoot = doc.RootElement();

        XMLElement* existing = cRoot->FirstChildElement("Tag");
        if (existing) cRoot->DeleteChild(existing);

        XMLElement* tagElement = doc.NewElement("Tag");
        tagElement->SetAttribute("value", tag.c_str());
        cRoot->InsertEndChild(tagElement);

        return doc.SaveFile(path.c_str());
    }

    static std::string readActorTag(const std::string& actorName)
    {
        std::string path = BuildPath("Game Objects/" + SceneGraph::getInstance().sceneFileName + "/" + actorName + ".xml").string();

        XMLDocument doc;

        if (doc.LoadFile(path.c_str()) != XML_SUCCESS) return "Untagged";

        XMLNode* cRoot = doc.RootElement();
        if (!cRoot) return "Untagged";

        XMLElement* tagElement = cRoot->FirstChildElement("Tag");
        if (!tagElement) return "Untagged";

        const char* val = tagElement->Attribute("value");
        return val ? std::string(val) : "Untagged";
    }

    // instead of putting all the light shadow settings into the oncreate and cluttering it, creating a seperate function that applys it
    static void applyLightShadowSettings(const std::string& name, Ref<LightComponent> lc) {
        if (!lc) return;

        std::string path = BuildPath("Game Objects/" + SceneGraph::getInstance().sceneFileName + "/" + name + ".xml").string();

        XMLDocument doc;
        if (doc.LoadFile(path.c_str()) != XML_SUCCESS) return;

        XMLNode* cRoot = doc.RootElement();
        if (!cRoot) return;

        XMLElement* componentElement = cRoot->FirstChildElement("LightComponent");
        if (!componentElement) return;

        // lambda helper to get value, with a fallback incase things break
        auto getValue = [](XMLElement* el, const char* attr, float fallback = 0.0f) -> float {
            if (!el) return fallback;
            const XMLAttribute* a = el->FindAttribute(attr);
            return a ? a->FloatValue() : fallback;
            };

        XMLElement* shadowTypeEl = componentElement->FirstChildElement("shadowType");
        XMLElement* shadowNearEl = componentElement->FirstChildElement("shadowNear");
        XMLElement* shadowFarEl = componentElement->FirstChildElement("shadowFar");
        XMLElement* shadowResEl = componentElement->FirstChildElement("shadowResolution");
        XMLElement* shadowOrthoEl = componentElement->FirstChildElement("shadowOrthoSize");

        // applying the values, providing default to fallback incase lights don't have the new settings
        if (shadowTypeEl) lc->setShadowType(static_cast<ShadowType>((int)getValue(shadowTypeEl, "type", 0.0f)));
        if (shadowNearEl) lc->setShadowNear(getValue(shadowNearEl, "value", 0.1f));
        if (shadowFarEl) lc->setShadowFar(getValue(shadowFarEl, "value", 200.0f));
        if (shadowResEl) lc->setShadowResolution((int)getValue(shadowResEl, "value", 1024.0f));
        if (shadowOrthoEl) lc->setShadowOrthoSize(getValue(shadowOrthoEl, "value", 60.0f));
    }

    ///Adds actors from requested cell filename into sceneGraph
    static int addActorsFromFile(SceneGraph* sceneGraph, std::string filename) {
        std::string path = BuildPath("Scenes/" + filename + ".scene").string();

        const char* id = path.c_str();
        XMLDocument doc;

        //try loading the file into doc
        XMLError eResult = doc.LoadFile(id);
        if (eResult != XML_SUCCESS) {
#ifdef _DEBUG
            std::cerr << "Error loading file " << id << ": " << eResult << std::endl;
#endif
            return eResult;
        }
        XMLNode* cRoot = doc.RootElement();

        XMLElement* actorList = cRoot->FirstChildElement("Actors");
        //XMLElement* newElementList = actorList->NextSiblingElement("Test");


        //Loop through each element and add it 


        runCreateActorsOfElementChildren(sceneGraph, actorList);

        return 1;
    }

    static int writeActorToCell(std::string filename, Ref<Actor> actor_, bool enabled) {
        std::string path = BuildPath("Scenes/" + filename + ".scene").string();
        const char* id = path.c_str();
        XMLDocument doc;



        //try loading the file into doc
        XMLError eResult = doc.LoadFile(id);
        if (eResult != XML_SUCCESS) {
#ifdef _DEBUG
            std::cerr << "Error loading file " << id << ": " << eResult << std::endl;
#endif
            return eResult;
        }


        std::cout << "Found file to write: " << filename << std::endl;

        XMLNode* cRoot = doc.RootElement();



        XMLElement* actorList = cRoot->FirstChildElement("Actors");

        //Create the Actors element if no actors were created previously before
        if (!actorList) {
            actorList = doc.NewElement("Actors");

            std::cout << "Creating Element: Actors" << std::endl;

        }

        cRoot->InsertEndChild(actorList);

        //return shouldn't matter as its already attached.
        writeActorRecursive(actor_.get(), actorList);



        //string value for name as function return cannot be made into a cstr
        std::string nameStr = actor_->getActorName();
        //make cstr name to be set as name of element
        const char* nameCStr = nameStr.c_str();

        //If actor has a parent, call recursion for

        doc.Print();

        XMLError eResultSave = doc.SaveFile(id);

        if (eResultSave != XML_SUCCESS) {
#ifdef _DEBUG
            std::cerr << "Error saving file: " << eResultSave << std::endl;
#endif
            return -1;
        }

        std::cout << nameCStr << "obj Save game written to '" << filename << ".xml'\n";

        return 0;

    }


    template<typename ComponentTemplate>
    static auto getComponent(std::string name, int copy = 0) {

        /*

        example of use

        TransformComponent* tempTestWrite = std::apply([](auto&&... args) {
        return new TransformComponent(args...);
        }, XMLtest::getComponent<TransformComponent>("Bob"));

        */

        std::string path = BuildPath("Game Objects/" + SceneGraph::getInstance().sceneFileName + "/" + name + ".xml").string();
        const char* id = path.c_str();

        XMLDocument doc;

        std::string componentType = static_cast<std::string>(typeid(ComponentTemplate).name()).substr(6);


        XMLError eResult = doc.LoadFile(id);
        if (eResult != XML_SUCCESS) {
#ifdef _DEBUG
            std::cerr << "Error loading file " << id << ": " << eResult << std::endl;
#endif
        }

        XMLNode* cRoot = doc.RootElement();

        XMLElement* component = cRoot->FirstChildElement(componentType.c_str());

        //Specify the components to determine how to extract data for each type
        if constexpr (std::is_same_v<ComponentTemplate, TransformComponent>) {
            //TRANSFORM

            XMLElement* position = component->FirstChildElement("position");
            XMLElement* rotation = component->FirstChildElement("rotation");
            XMLElement* scale = component->FirstChildElement("scale");

            //TRANSFORM POSITION
            Vec3 posArg = Vec3(
                GetAttrF(position, "x"),
                GetAttrF(position, "y"),
                GetAttrF(position, "z")
            );

            //TRANSFORM ROTATION
            Quaternion rotationArg = Quaternion(
                GetAttrF(rotation, "w"),
                Vec3(
                    GetAttrF(rotation, "x"),
                    GetAttrF(rotation, "y"),
                    GetAttrF(rotation, "z")
                )
            );

            //TRANSFORM SCALE
            Vec3 scaleArg = Vec3(
                GetAttrF(scale, "x"),
                GetAttrF(scale, "y"),
                GetAttrF(scale, "z")

            );

            //return the tuple to act as arguments
            auto args = std::make_tuple(posArg,
                rotationArg,
                scaleArg);
            return args;

        }
        else if constexpr (std::is_same_v<ComponentTemplate, PhysicsComponent>) {
            // physics

            XMLElement* stateElement = component->FirstChildElement("PhysicsState");
            XMLElement* constraintsElement = component->FirstChildElement("Constraints");
            XMLElement* massElement = component->FirstChildElement("mass");
            XMLElement* useGravityElement = component->FirstChildElement("UsingGravity");
            XMLElement* dragElement = component->FirstChildElement("drag");
            XMLElement* angularDragElement = component->FirstChildElement("angularDrag");
            XMLElement* frictionElement = component->FirstChildElement("friction");
            XMLElement* restitutionElement = component->FirstChildElement("restitution");

            // physics state
            PhysicsState physicsStateArg = static_cast<PhysicsState>(GetAttrF(stateElement, "state"));

            // constraints
            PhysicsConstraints constraintsArg;
            constraintsArg.freezePosX = static_cast<bool>(GetAttrF(constraintsElement, "freezePosX"));
            constraintsArg.freezePosY = static_cast<bool>(GetAttrF(constraintsElement, "freezePosY"));
            constraintsArg.freezePosZ = static_cast<bool>(GetAttrF(constraintsElement, "freezePosZ"));
            constraintsArg.freezeRotX = static_cast<bool>(GetAttrF(constraintsElement, "freezeRotX"));
            constraintsArg.freezeRotY = static_cast<bool>(GetAttrF(constraintsElement, "freezeRotY"));
            constraintsArg.freezeRotZ = static_cast<bool>(GetAttrF(constraintsElement, "freezeRotZ"));

            // mass
            float massArg = GetAttrF(massElement, "value");

            // using gravity
            bool useGravityArg = static_cast<bool>(GetAttrF(useGravityElement, "isUsing"));

            // drag
            float dragArg = GetAttrF(dragElement, "value");

            // angular drag
            float angularDragArg = GetAttrF(angularDragElement, "value");

            // friciton
            float frictionArg = GetAttrF(frictionElement, "value");

            // restitution
            float restitutionArg = GetAttrF(restitutionElement, "value");

            //return the tuple to act as arguments
            auto args = std::make_tuple(nullptr,
                physicsStateArg,
                constraintsArg,
                massArg,
                useGravityArg,
                dragArg,
                angularDragArg,
                frictionArg,
                restitutionArg
            );
            return args;
        }
        else if constexpr (std::is_same_v<ComponentTemplate, CollisionComponent>) {
            // collision

            XMLElement* stateElement = component->FirstChildElement("CollisionState");
            XMLElement* typeElement = component->FirstChildElement("CollisionType");
            XMLElement* isTriggerElement = component->FirstChildElement("isTrigger");
            XMLElement* radiusElement = component->FirstChildElement("radius");
            XMLElement* centreElement = component->FirstChildElement("centre");
            XMLElement* centrePosAElement = component->FirstChildElement("centrePosA");
            XMLElement* centrePosBElement = component->FirstChildElement("centrePosB");
            XMLElement* halfExtentsElement = component->FirstChildElement("halfExtents");
            XMLElement* orientationElement = component->FirstChildElement("orientation");

            // collision state
            ColliderState collisionStateArg = static_cast<ColliderState>(GetAttrF(stateElement, "state"));

            // collision state
            ColliderType typeArg = static_cast<ColliderType>(GetAttrF(typeElement, "type"));

            // is trigger
            bool isTriggerArg = static_cast<bool>(GetAttrF(isTriggerElement, "trigger"));

            // radius
            float radiusArg = GetAttrF(radiusElement, "value");

            // centre
            Vec3 centreArg = Vec3(
                GetAttrF(centreElement, "x"),
                GetAttrF(centreElement, "y"),
                GetAttrF(centreElement, "z")
            );

            // centrePosA
            Vec3 centrePosAArg = Vec3(
                GetAttrF(centrePosAElement, "x"),
                GetAttrF(centrePosAElement, "y"),
                GetAttrF(centrePosAElement, "z")
            );

            // centrePosB
            Vec3 centrePosBArg = Vec3(
                GetAttrF(centrePosBElement, "x"),
                GetAttrF(centrePosBElement, "y"),
                GetAttrF(centrePosBElement, "z")
            );

            // half extents
            Vec3 halfExtentsArg = Vec3(
                GetAttrF(halfExtentsElement, "x"),
                GetAttrF(halfExtentsElement, "y"),
                GetAttrF(halfExtentsElement, "z")
            );

            // orientation
            Quaternion orientationArg = Quaternion(
                GetAttrF(orientationElement, "w"),
                Vec3(
                    GetAttrF(orientationElement, "x"),
                    GetAttrF(orientationElement, "y"),
                    GetAttrF(orientationElement, "z")
                )
            );

            //return the tuple to act as arguments
            auto args = std::make_tuple(nullptr,
                collisionStateArg,
                typeArg,
                isTriggerArg,
                radiusArg,
                centreArg,
                centrePosAArg,
                centrePosBArg,
                halfExtentsArg,
                orientationArg
            );
            return args;
        }
        else if constexpr (std::is_same_v<ComponentTemplate, CameraComponent>) {
            XMLElement* fovElement = component->FirstChildElement("fov");
            XMLElement* nearElement = component->FirstChildElement("near");
            XMLElement* farElement = component->FirstChildElement("far");
            XMLElement* typeElement = component->FirstChildElement("type");
            XMLElement* orthoSizeElement = component->FirstChildElement("orthoSize");

            // type (giving default value so it doesnt break everything)
            ProjectionType typeArg = typeElement ? static_cast<ProjectionType>(typeElement->FindAttribute("value")->IntValue()) : ProjectionType::Perspective;

            // fov
            float fovArg = GetAttrF(fovElement, "value");

            // near
            float nearArg = GetAttrF(nearElement, "value");

            // far
            float farArg = GetAttrF(farElement, "value");

            // orthoSize (giving default value so it doesnt break everything)
            float orthoSizeArg = orthoSizeElement ? GetAttrF(orthoSizeElement, "value") : 5.0f;

            //return the tuple to act as arguments
            auto args = std::make_tuple(nullptr,
                typeArg,
                fovArg,
                nearArg,
                farArg,
                orthoSizeArg
            );
            return args;

        }
        else if constexpr (std::is_same_v<ComponentTemplate, LightComponent>) {
            //LIGHT

            XMLElement* diffElement = component->FirstChildElement("diffuse");
            XMLElement* specElement = component->FirstChildElement("specular");
            XMLElement* intensityElement = component->FirstChildElement("intensity");
            XMLElement* typeElement = component->FirstChildElement("LightType");

            //LIGHT DIFF
            Vec4 diffArg = Vec4(
                GetAttrF(diffElement, "x"),
                GetAttrF(diffElement, "y"),
                GetAttrF(diffElement, "z"),
                GetAttrF(diffElement, "w")
            );

            //LIGHT SPEC
            Vec4 specArg = Vec4(
                GetAttrF(specElement, "x"),
                GetAttrF(specElement, "y"),
                GetAttrF(specElement, "z"),
                GetAttrF(specElement, "w")
            );

            //LIGHT INTENSITY
            float intensityArg = GetAttrF(intensityElement, "magnitude");

            //ENUM TYPE
            LightType lightTypeArg = static_cast<LightType>(GetAttrF(typeElement, "type"));


            //return the tuple to act as arguments
            auto args = std::make_tuple(nullptr,
                lightTypeArg,
                specArg,
                diffArg,
                intensityArg
            );
            return args;

        }
        else if constexpr (std::is_same_v<ComponentTemplate, AnimatorComponent>) {
            //Animator

            XMLElement* startTimeElement = component->FirstChildElement("startTime");
            XMLElement* speedMultElement = component->FirstChildElement("speedMult");
            XMLElement* loopingElement = component->FirstChildElement("looping");
            XMLElement* animNameElement = component->FirstChildElement("animName");

            float startTimeArg = GetAttrF(startTimeElement, "time");

            float speedMultArg = GetAttrF(speedMultElement, "mult");

            bool loopingArg = GetAttrF(loopingElement, "state");

            std::string nameArg = std::string(animNameElement->FindAttribute("name")->Value());



            //return the tuple to act as arguments
            auto args = std::make_tuple(
                startTimeArg,
                speedMultArg,
                loopingArg,
                nameArg
            );
            return args;

        }
        else if constexpr (std::is_same_v<ComponentTemplate, TilingSettings>) {
            /////// TILE SETTINGS ///////

            XMLElement* isTiled = component->FirstChildElement("isTiled");
            XMLElement* tilingScale = component->FirstChildElement("tileScale");
            XMLElement* tilingOffset = component->FirstChildElement("tileOffset");

            bool isTiledArg = GetAttrF(isTiled, "state");

            Vec2 tilingScaleArgs = Vec2(
                GetAttrF(tilingScale, "x"),
                GetAttrF(tilingScale, "y")
            );


            Vec2 tilingOffsetArgs = Vec2(
                GetAttrF(tilingOffset, "x"),
                GetAttrF(tilingOffset, "y")
            );

            auto args = std::make_tuple(
                isTiledArg,
                tilingScaleArgs,
                tilingOffsetArgs
            );

            return args;
        }
        else if constexpr (std::is_same_v<ComponentTemplate, ShadowSettings>) {
            /////// TILE SETTINGS ///////

            XMLElement* castShadow = component->FirstChildElement("castShadow");


            bool castShadowArg = GetAttrF(castShadow, "state");



            auto args = std::make_tuple(
                castShadowArg
            );

            return args;
        }
        else if constexpr (std::is_same_v<ComponentTemplate, MeshComponent> || std::is_same_v<ComponentTemplate, MaterialComponent> || std::is_same_v<ComponentTemplate, ShaderComponent> || std::is_same_v<ComponentTemplate, ScriptComponent>) {

            AssetManager& assetMgr = AssetManager::getInstance();
            
            if (copy > 0) {
                //when you want a different copy, find that
                if (cRoot->ChildElementCount(componentType.c_str()) > copy) {
                    
                    for (int i = 0; i < copy && component != nullptr; i++) {
                        component = component->NextSiblingElement(componentType.c_str());
                    }
                }
                else { 
                    
                    return std::string(); 
                }                
            }
            //return the asset reference in AssetManager named in element
            std::string assetName = component->FindAttribute("name")->Value();
            
            const char* assetNameCStr = assetName.c_str();

            auto args = assetName;
            return args;



        }

    }

    static inline std::vector<float> getPublicVars(std::string name, int copy = 0) {
        std::string path = BuildPath("Game Objects/" + SceneGraph::getInstance().sceneFileName + "/" + name + ".xml").string();
        const char* id = path.c_str();

        XMLDocument doc;

        std::string componentType = static_cast<std::string>(typeid(ScriptComponent).name()).substr(6);


        XMLError eResult = doc.LoadFile(id);
        if (eResult != XML_SUCCESS) {
#ifdef _DEBUG
            std::cerr << "Error loading file " << id << ": " << eResult << std::endl;
#endif
        }

        XMLNode* cRoot = doc.RootElement();

        XMLElement* component = cRoot->FirstChildElement(componentType.c_str());


        if (copy > 0) {
            //when you want a different copy, find that
            if (cRoot->ChildElementCount(componentType.c_str()) > copy) {

                for (int i = 0; i < copy && component != nullptr; i++) {
                    component = component->NextSiblingElement(componentType.c_str());
                }
            }
            else {

                return std::vector<float>{};
            }
        }

        std::vector<float> pubVars;

        int index = 0;
        while (true) {
            std::string strIndexName = "e" + std::to_string(index);
            const char* cStrIndex = strIndexName.c_str();

            if (component->FindAttribute(cStrIndex) != nullptr) {
                float var = component->FindAttribute(cStrIndex)->FloatValue();
                pubVars.push_back(var);
            }
            else { break; }
            
            index++;
        }

        //return the asset reference in AssetManager named in element
        return pubVars;
    }

    ///Simplify FindAttribute() implimentation for coding convenience
    static inline float GetAttrF(XMLElement* element, const char* name) {
        return element->FindAttribute(name)->FloatValue();
    }

    template<typename ComponentTemplate>
    static int writeUniqueComponent(std::string name, Component* toWrite) {
        std::string path = BuildPath("Game Objects/" + SceneGraph::getInstance().sceneFileName + "/" + name + ".xml").string();
        const char* id = path.c_str();
        XMLDocument doc;
        
        //try loading the file into doc
        XMLError eResult = doc.LoadFile(id);
        if (eResult != XML_SUCCESS) {
#ifdef _DEBUG
            std::cerr << "Error loading file " << id << ": " << eResult << std::endl;
#endif
            return eResult;
        }


        std::cout << "Found file to write: " << name << std::endl;


        //the basic format for any component
        XMLNode* cRoot = doc.RootElement();
        std::string componentType;

        componentType = static_cast<std::string>(typeid(ComponentTemplate).name()).substr(6);

        XMLElement* componentElement = cRoot->FirstChildElement(componentType.c_str());
        if (componentElement) {
            std::cout << "Found component (Deleting): " << name << std::endl;

            componentElement->DeleteChildren();
        }
        std::cout << "Creating Element: " << componentType << std::endl;


        //The element for the new component you are adding
        componentElement = doc.NewElement(componentType.c_str());


        if constexpr (std::is_same_v<ComponentTemplate, TransformComponent>) {
            TransformComponent* componentToWrite = dynamic_cast<TransformComponent*>(toWrite);

            //POSITION
            XMLElement* position;
            position = doc.NewElement("position");

            Vec3 pos = componentToWrite->GetPosition();
            position->SetAttribute("x", pos.x);
            position->SetAttribute("y", pos.y);
            position->SetAttribute("z", pos.z);

            componentElement->InsertEndChild(position);

            //ROTATION
            XMLElement* rotation;
            rotation = doc.NewElement("rotation");

            Quaternion rot = componentToWrite->GetOrientation();

            rotation->SetAttribute("w", rot.w);
            rotation->SetAttribute("x", rot.ijk.x);
            rotation->SetAttribute("y", rot.ijk.y);
            rotation->SetAttribute("z", rot.ijk.z);

            componentElement->InsertEndChild(rotation);

            //SCALE
            XMLElement* scale;
            scale = doc.NewElement("scale");

            Vec3 scaleVector = componentToWrite->GetScale();
            scale->SetAttribute("x", scaleVector.x);
            scale->SetAttribute("y", scaleVector.y);
            scale->SetAttribute("z", scaleVector.z);

            componentElement->InsertEndChild(scale);
        }
        else if constexpr (std::is_same_v<ComponentTemplate, PhysicsComponent>) {
            PhysicsComponent* componentToWrite = dynamic_cast<PhysicsComponent*>(toWrite);

            // physics state
            XMLElement* stateElement;
            stateElement = doc.NewElement("PhysicsState");
            stateElement->SetAttribute("state", static_cast<int>(componentToWrite->getState()));
            componentElement->InsertEndChild(stateElement);

            // constraints
            XMLElement* constraintsElement;
            constraintsElement = doc.NewElement("Constraints");
            const PhysicsConstraints& constraints = componentToWrite->getConstraints();
            constraintsElement->SetAttribute("freezePosX", static_cast<int>(constraints.freezePosX));
            constraintsElement->SetAttribute("freezePosY", static_cast<int>(constraints.freezePosY));
            constraintsElement->SetAttribute("freezePosZ", static_cast<int>(constraints.freezePosZ));
            constraintsElement->SetAttribute("freezeRotX", static_cast<int>(constraints.freezeRotX));
            constraintsElement->SetAttribute("freezeRotY", static_cast<int>(constraints.freezeRotY));
            constraintsElement->SetAttribute("freezeRotZ", static_cast<int>(constraints.freezeRotZ));
            componentElement->InsertEndChild(constraintsElement);

            // mass
            XMLElement* massElement;
            massElement = doc.NewElement("mass");
            massElement->SetAttribute("value", componentToWrite->getMass());
            componentElement->InsertEndChild(massElement);

            // using gravity
            XMLElement* useGravityElement;
            useGravityElement = doc.NewElement("UsingGravity");
            useGravityElement->SetAttribute("isUsing", static_cast<int>(componentToWrite->getUseGravity()));
            componentElement->InsertEndChild(useGravityElement);

            // drag
            XMLElement* dragElement;
            dragElement = doc.NewElement("drag");
            dragElement->SetAttribute("value", componentToWrite->getDrag());
            componentElement->InsertEndChild(dragElement);

            // angular drag
            XMLElement* angularDragElement;
            angularDragElement = doc.NewElement("angularDrag");
            angularDragElement->SetAttribute("value", componentToWrite->getAngularDrag());
            componentElement->InsertEndChild(angularDragElement);

            // friction
            XMLElement* frictionElement;
            frictionElement = doc.NewElement("friction");
            frictionElement->SetAttribute("value", componentToWrite->getFriction());
            componentElement->InsertEndChild(frictionElement);

            // restitution
            XMLElement* restitutionElement;
            restitutionElement = doc.NewElement("restitution");
            restitutionElement->SetAttribute("value", componentToWrite->getRestitution());
            componentElement->InsertEndChild(restitutionElement);
        }
        else if constexpr (std::is_same_v<ComponentTemplate, CollisionComponent>) {
            CollisionComponent* componentToWrite = dynamic_cast<CollisionComponent*>(toWrite);

            // collision state
            XMLElement* stateElement;
            stateElement = doc.NewElement("CollisionState");
            stateElement->SetAttribute("state", static_cast<int>(componentToWrite->getState()));
            componentElement->InsertEndChild(stateElement);

            // collision type
            XMLElement* typeElement;
            typeElement = doc.NewElement("CollisionType");
            typeElement->SetAttribute("type", static_cast<int>(componentToWrite->getType()));
            componentElement->InsertEndChild(typeElement);

            // is trigger
            XMLElement* isTriggerElement;
            isTriggerElement = doc.NewElement("isTrigger");
            isTriggerElement->SetAttribute("trigger", static_cast<int>(componentToWrite->getIsTrigger()));
            componentElement->InsertEndChild(isTriggerElement);

            // radius
            XMLElement* radiusElement;
            radiusElement = doc.NewElement("radius");
            radiusElement->SetAttribute("value", componentToWrite->getRadius());
            componentElement->InsertEndChild(radiusElement);

            // centre
            XMLElement* centre;
            centre = doc.NewElement("centre");

            Vec3 vCentre = componentToWrite->getCentre();
            centre->SetAttribute("x", vCentre.x);
            centre->SetAttribute("y", vCentre.y);
            centre->SetAttribute("z", vCentre.z);

            componentElement->InsertEndChild(centre);

            // centrePosA
            XMLElement* centrePosA;
            centrePosA = doc.NewElement("centrePosA");

            Vec3 vCentrePosA = componentToWrite->getCentrePosA();
            centrePosA->SetAttribute("x", vCentrePosA.x);
            centrePosA->SetAttribute("y", vCentrePosA.y);
            centrePosA->SetAttribute("z", vCentrePosA.z);

            componentElement->InsertEndChild(centrePosA);

            // centrePosB
            XMLElement* centrePosB;
            centrePosB = doc.NewElement("centrePosB");

            Vec3 vcentrePosB = componentToWrite->getCentrePosB();
            centrePosB->SetAttribute("x", vcentrePosB.x);
            centrePosB->SetAttribute("y", vcentrePosB.y);
            centrePosB->SetAttribute("z", vcentrePosB.z);

            componentElement->InsertEndChild(centrePosB);

            // half extents
            XMLElement* halfExtents;
            halfExtents = doc.NewElement("halfExtents");

            Vec3 vhalfExtents = componentToWrite->getHalfExtents();
            halfExtents->SetAttribute("x", vhalfExtents.x);
            halfExtents->SetAttribute("y", vhalfExtents.y);
            halfExtents->SetAttribute("z", vhalfExtents.z);

            componentElement->InsertEndChild(halfExtents);

            // orientation
            XMLElement* orientation;
            orientation = doc.NewElement("orientation");

            Quaternion ori = componentToWrite->getOrientation();

            orientation->SetAttribute("w", ori.w);
            orientation->SetAttribute("x", ori.ijk.x);
            orientation->SetAttribute("y", ori.ijk.y);
            orientation->SetAttribute("z", ori.ijk.z);

            componentElement->InsertEndChild(orientation);
        }
        else if constexpr (std::is_same_v<ComponentTemplate, CameraComponent>) {
            CameraComponent* componentToWrite = dynamic_cast<CameraComponent*>(toWrite);

            // projection type
            XMLElement* typeElement;
            typeElement = doc.NewElement("type");
            typeElement->SetAttribute("value", static_cast<int>(componentToWrite->getType()));
            componentElement->InsertEndChild(typeElement);

            // fov
            XMLElement* fovElement;
            fovElement = doc.NewElement("fov");
            fovElement->SetAttribute("value", componentToWrite->getFOV());
            componentElement->InsertEndChild(fovElement);

            // near
            XMLElement* nearElement;
            nearElement = doc.NewElement("near");
            nearElement->SetAttribute("value", componentToWrite->getNearClipPlane());
            componentElement->InsertEndChild(nearElement);

            // far
            XMLElement* farElement;
            farElement = doc.NewElement("far");
            farElement->SetAttribute("value", componentToWrite->getFarClipPlane());
            componentElement->InsertEndChild(farElement);

            // ortho size
            XMLElement* orthoSizeElement;
            orthoSizeElement = doc.NewElement("orthoSize");
            orthoSizeElement->SetAttribute("value", componentToWrite->getOrthoSize());
            componentElement->InsertEndChild(orthoSizeElement);
        }
        else if constexpr (std::is_same_v<ComponentTemplate, LightComponent>) {
            LightComponent* componentToWrite = dynamic_cast<LightComponent*>(toWrite);
            
            Vec4 diff = componentToWrite->getDiff();
            Vec4 spec = componentToWrite->getSpec();
            GLfloat intensity = componentToWrite->getIntensity();
            LightType lightType = componentToWrite->getType();

            //diffuse
            XMLElement* diffElement;
            diffElement = doc.NewElement("diffuse");
            {
                diffElement->SetAttribute("x", diff.x);
                diffElement->SetAttribute("y", diff.y);
                diffElement->SetAttribute("z", diff.z);
                diffElement->SetAttribute("w", diff.w);
            }
            componentElement->InsertEndChild(diffElement);

            //specular
            XMLElement* specElement;
            specElement = doc.NewElement("specular");
            {
                specElement->SetAttribute("x", spec.x);
                specElement->SetAttribute("y", spec.y);
                specElement->SetAttribute("z", spec.z);
                specElement->SetAttribute("w", spec.w);
            }
            componentElement->InsertEndChild(specElement);

            //intensity
            XMLElement* intensityElement;
            intensityElement = doc.NewElement("intensity");
            {
                intensityElement->SetAttribute("magnitude", static_cast<float>(intensity));
            }
            componentElement->InsertEndChild(intensityElement);

            //LightType
            XMLElement* typeElement;
            typeElement = doc.NewElement("LightType");
            {
                typeElement->SetAttribute("type", static_cast<int>(lightType));
            }
            componentElement->InsertEndChild(typeElement);

            // shadowType
            XMLElement* shadowTypeElement = doc.NewElement("shadowType");
            shadowTypeElement->SetAttribute("type", static_cast<int>(componentToWrite->getShadowType()));
            componentElement->InsertEndChild(shadowTypeElement);

            // shadowNear
            XMLElement* shadowNearElement = doc.NewElement("shadowNear");
            shadowNearElement->SetAttribute("value", componentToWrite->getShadowNear());
            componentElement->InsertEndChild(shadowNearElement);

            // shadowFar
            XMLElement* shadowFarElement = doc.NewElement("shadowFar");
            shadowFarElement->SetAttribute("value", componentToWrite->getShadowFar());
            componentElement->InsertEndChild(shadowFarElement);

            // shadowResolution
            XMLElement* shadowResElement = doc.NewElement("shadowResolution");
            shadowResElement->SetAttribute("value", componentToWrite->getShadowResolution());
            componentElement->InsertEndChild(shadowResElement);

            // shadowOrthoSize
            XMLElement* shadowOrthoElement = doc.NewElement("shadowOrthoSize");
            shadowOrthoElement->SetAttribute("value", componentToWrite->getShadowOrthoSize());
            componentElement->InsertEndChild(shadowOrthoElement);
        }
        else if constexpr (std::is_same_v<ComponentTemplate, AnimatorComponent>) {
            AnimatorComponent* componentToWrite = dynamic_cast<AnimatorComponent*>(toWrite);

            float startTime = componentToWrite->getStartTime();
            float speedMult = componentToWrite->getSpeedMult();
            bool loop = componentToWrite->getLoopingState();

            std::string animName = "";

            if (componentToWrite->hasAnim()) animName = componentToWrite->getAnimName();

            //startTime
            XMLElement* startTimeElement;
            startTimeElement = doc.NewElement("startTime");
            {
                startTimeElement->SetAttribute("time", startTime);
            }
            componentElement->InsertEndChild(startTimeElement);

            //speedMult
            XMLElement* speedMultElement;
            speedMultElement = doc.NewElement("speedMult");
            {
                speedMultElement->SetAttribute("mult", speedMult);
                
            }
            componentElement->InsertEndChild(speedMultElement);

            //loopingState
            XMLElement* loopElement;
            loopElement = doc.NewElement("looping");
            {
                loopElement->SetAttribute("state", loop);
            }
            componentElement->InsertEndChild(loopElement);

            //filename
            XMLElement* animNameElement;
            animNameElement = doc.NewElement("animName");
            {
                animNameElement->SetAttribute("name", animName.c_str());
            }
            componentElement->InsertEndChild(animNameElement);

            }
        else if constexpr (std::is_same_v<ComponentTemplate, ShadowSettings>) {
            ShadowSettings* shadowToWrite = dynamic_cast<ShadowSettings*>(toWrite);

            bool castShadow = shadowToWrite->getCastShadow();

            // isTiled
            XMLElement* isShadowElement;
            isShadowElement = doc.NewElement("castShadow");
            {
                isShadowElement->SetAttribute("state", (int)castShadow);
            }
            componentElement->InsertEndChild(isShadowElement);

        }
        // Material Tiling
        else if constexpr (std::is_same_v<ComponentTemplate, TilingSettings>) {
            TilingSettings* tilingToWrite = dynamic_cast<TilingSettings*>(toWrite);
        
            bool isTiled = tilingToWrite->getIsTiled();
            Vec2 tilingScale = tilingToWrite->getTileScale();
            Vec2 tilingOffset = tilingToWrite->getTileOffset();

            // isTiled
            XMLElement* isTiledElement;
            isTiledElement = doc.NewElement("isTiled");
            {
                isTiledElement->SetAttribute("state", (int)isTiled);
            }
            componentElement->InsertEndChild(isTiledElement);

            // tileScale
            XMLElement* tilingScaleElement;
            tilingScaleElement = doc.NewElement("tileScale");
            {
                tilingScaleElement->SetAttribute("x", tilingScale.x);
                tilingScaleElement->SetAttribute("y", tilingScale.y);
            }
            componentElement->InsertEndChild(tilingScaleElement);

            // tileOffset
            XMLElement* tilingOffsetElement;
            tilingOffsetElement = doc.NewElement("tileOffset");
            {
                tilingOffsetElement->SetAttribute("x", tilingOffset.x);
                tilingOffsetElement->SetAttribute("y", tilingOffset.y);
            }
            componentElement->InsertEndChild(tilingOffsetElement);
        }
        else {

            std::cout << "ComponentWriteError: " << componentType << " is not a supported component type" << std::endl;
        }


        cRoot->InsertEndChild(componentElement);
        doc.Print();

        XMLError eResultSave = doc.SaveFile(id);

        if (eResultSave != XML_SUCCESS) {
#ifdef _DEBUG
            std::cerr << "Error saving file: " << eResultSave << std::endl;
#endif
            return -1;
        }

        std::cout << "Save game written to '" << name << ".xml'\n";

        return 0;
    }

    /// <summary>
    /// Used to write the specific asset's name/key as an actor's used component in XML file. 
    /// NOT USED FOR TRANSFORM COMPONENT OR PHYSICS COMPONENT, YOU ARE LOOKING FOR writeUniqueComponent()
    /// </summary>
    /// <typeparam name="ComponentTemplate"></typeparam>
    /// <param name="name">name of the actor</param>
    /// <param name="toWrite">component to be written</param>
    /// <returns></returns>
    template<typename ComponentTemplate>
    static int writeReferenceComponent(std::string name, Ref<Component> toWrite); 

    /// <summary>
    /// Used to check if an actor has a specific type of component in their XML file
    /// </summary>
    /// <typeparam name="ComponentTemplate"></typeparam>
    /// <param name="name">name of the actor</param>
    /// <returns>true if the component type exists within the XML file</returns>
    template<typename ComponentTemplate>
    static bool hasComponent(std::string name) {
        std::string path = BuildPath("Game Objects/" + SceneGraph::getInstance().sceneFileName + "/" + name + ".xml").string();
        const char* id = path.c_str();

        XMLDocument doc;
        //try loading the file into doc
        XMLError eResult = doc.LoadFile(id);
        if (eResult != XML_SUCCESS) {
#ifdef _DEBUG
            std::cerr << "Error loading file " << id << ": " << eResult << std::endl;
#endif
            return false;
        }
        
        XMLNode* cRoot = doc.RootElement();
        if (!cRoot) {
            return false;
        }

        std::string componentType = static_cast<std::string>(typeid(ComponentTemplate).name()).substr(6);
        XMLElement* component = cRoot->FirstChildElement(componentType.c_str());

        return (component != nullptr);
    }
};

//Requires XMLManager to exist
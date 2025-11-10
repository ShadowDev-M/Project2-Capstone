#pragma once
#include <iostream>
#include <string>

#include "tinyxml2.h"
#include "Component.h"
#include <tuple>
#include "TransformComponent.h"  
#include "CameraComponent.h"  
#include "LightComponent.h"  

#include "SceneGraph.h"


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

    static void addAttributeRecursive(SceneGraph* sceneGraph, const XMLAttribute* attribute);


public:

    /// Creates an object file for the actor
    static int writeActor(std::string name) {

        XMLDocument doc;

        XMLNode* cRoot = doc.NewElement("Actor");
        doc.InsertFirstChild(cRoot);

        doc.SaveFile(("Game Objects/" + SceneGraph::getInstance().cellFileName + "/" + name + ".xml").c_str());
        return 0;
    }

    static int writeCellFile(std::string name) {

        XMLDocument doc;

        XMLNode* cRoot = doc.NewElement("CellFile");
        doc.InsertFirstChild(cRoot);

        doc.SaveFile(("Cell Files/" + name + ".xml").c_str());
        return 0;
    }

    ///Adds actors from requested cell filename into sceneGraph
    static int addActorsFromFile(SceneGraph* sceneGraph, std::string filename) {
        std::string path = "Cell Files/" + filename + ".xml";
        const char* id = path.c_str();
        XMLDocument doc;

        //try loading the file into doc
        XMLError eResult = doc.LoadFile(id);
        if (eResult != XML_SUCCESS) {
            std::cerr << "Error loading file " << id << ": " << eResult << std::endl;
            return eResult;
        }
        XMLNode* cRoot = doc.RootElement();

        XMLElement* actorList = cRoot->FirstChildElement("Actors");
        //XMLElement* newElementList = actorList->NextSiblingElement("Test");

        
        //Loop through each element and add it 

        
        runCreateActorsOfElementChildren(sceneGraph, actorList);


        //deprecated 
        //if (actorList->FirstAttribute()) addAttributeRecursive(sceneGraph, actorList->FirstAttribute());;
        return 1;





    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="sceneGraph"></param>
    /// <param name="filename"></param>
    /// <returns></returns>
    static int addComponentsFromFile(SceneGraph* sceneGraph, std::string filename) {
        std::string path = "Cell Files/" + filename + ".xml";
        const char* id = path.c_str();
        XMLDocument doc;

        //try loading the file into doc
        XMLError eResult = doc.LoadFile(id);
        if (eResult != XML_SUCCESS) {
            std::cerr << "Error loading file " << id << ": " << eResult << std::endl;
            return eResult;
        }
        XMLNode* cRoot = doc.RootElement();

        XMLElement* actorList = cRoot->FirstChildElement("Actors");


        if (actorList->FirstAttribute()) addAttributeRecursive(sceneGraph, actorList->FirstAttribute());;

    }

    static int writeComponentToCell(std::string filename, std::string name, std::string className, bool enabled) {

        AssetManager& assetMgr = AssetManager::getInstance();

        const char* nameCStr = name.c_str();
        const char* classCStr = className.c_str();


        std::string path = "Cell Files/" + filename + ".xml";
        const char* id = path.c_str();
        XMLDocument doc;



        //try loading the file into doc
        XMLError eResult = doc.LoadFile(id);
        if (eResult != XML_SUCCESS) {
            std::cerr << "Error loading file " << id << ": " << eResult << std::endl;
            return eResult;
        }


        std::cout << "Found file to write: " << filename << std::endl;

        XMLNode* cRoot = doc.RootElement();
        //just calling actors so it doesn't get wiped, xml is weird
        XMLElement* actors = cRoot->FirstChildElement("Actors");
        if (actors == nullptr) {
            std::cerr << "Root element not found!" << std::endl;
            return 1;
        }

        cRoot->InsertEndChild(actors);



        XMLElement* componentList;

        // Component Element 
        componentList = cRoot->FirstChildElement("Components");

        if (!componentList) {
            componentList = doc.NewElement("Components");

            std::cout << "Creating Element: Components" << std::endl;

        }
        {
            // Create the second element



            //std::cout << actorList->Name() << std::endl; 
        }

        {
            //Target Class
            XMLElement* targetComponentType;

            targetComponentType = componentList->FirstChildElement(classCStr);

            if (!targetComponentType) {
                targetComponentType = doc.NewElement(classCStr);

                std::cout << "Creating Element of Class: " << classCStr << std::endl;
            }

            {

                //Target Component Object
                XMLElement* targetComponent;

                targetComponent = componentList->FirstChildElement(nameCStr);

                if (!targetComponent) {
                    targetComponent = doc.NewElement(nameCStr);

                    std::cout << "Creating Element of Component Object: " << nameCStr << std::endl;
                }


                {

                    if (className == "MaterialComponent") {

                        // Create a new element for the Component Object
                        XMLElement* componentObjElement = doc.NewElement("Texture");

                        // Set the info as attributes
                        if (Ref<MaterialComponent> componentRef = assetMgr.GetAsset<MaterialComponent>(nameCStr)) {

                            if (componentRef->getTextureName()) componentObjElement->SetAttribute("path", componentRef->getTextureName());

                        }
                        else {
                            // No object is found
                            componentObjElement->SetAttribute("error", "NULL OBJECT");
                        }
                        // Attach the texture element to the MaterialComponent
                        targetComponent->InsertEndChild(componentObjElement);

                    }

                    if (className == "MeshComponent") {

                        // Create a new element for the Component Object
                        XMLElement* componentObjElement = doc.NewElement("Mesh");

                        // Set the info as attributes
                        if (Ref<MeshComponent> componentRef = assetMgr.GetAsset<MeshComponent>(nameCStr)) {

                            if (componentRef->getMeshName()) componentObjElement->SetAttribute("path", componentRef->getMeshName());

                        }
                        else {
                            // No object is found
                            componentObjElement->SetAttribute("error", "NULL OBJECT");
                        }

                        // Attach the texture element to the MaterialComponent
                        targetComponent->InsertEndChild(componentObjElement);

                    }
                    if (className == "ShaderComponent") {

                        // Create a new element for the Component Object
                        XMLElement* componentObjElement = doc.NewElement("Shader");

                        // Set the info as attributes
                        if (Ref<ShaderComponent> componentRef = assetMgr.GetAsset<ShaderComponent>(nameCStr)) {

                            if (componentRef->GetFragName()) {

                                componentObjElement->SetAttribute("frag", componentRef->GetFragName());
                                componentObjElement->SetAttribute("vert", componentRef->GetVertName());

                            }

                        }
                        else {
                            // No object is found
                            componentObjElement->SetAttribute("error", "NULL OBJECT");
                        }

                        // Attach the texture element to the MaterialComponent
                        targetComponent->InsertEndChild(componentObjElement);

                    }

                    


                };

                //Component Object end
                targetComponentType->InsertEndChild(targetComponent);


            };

            //Class end
            componentList->InsertEndChild(targetComponentType);
        };




        /* if (enabled) componentList->SetAttribute(nameCStr, nameCStr);
         else componentList->DeleteAttribute(nameCStr);*/


         //Component End
        cRoot->InsertEndChild(componentList);

        doc.Print();

        XMLError eResultSave = doc.SaveFile(id);

        if (eResultSave != XML_SUCCESS) {
            std::cerr << "Error saving file: " << eResultSave << std::endl;
            return -1;
        }

        std::cout << nameCStr << "obj Save game written to '" << filename << ".xml'\n";

        return 0;

    }


    static int writeActorToCell(std::string filename, Ref<Actor> actor_, bool enabled) {
        std::string path = "Cell Files/" + filename + ".xml";
        const char* id = path.c_str();
        XMLDocument doc;



        //try loading the file into doc
        XMLError eResult = doc.LoadFile(id);
        if (eResult != XML_SUCCESS) {
            std::cerr << "Error loading file " << id << ": " << eResult << std::endl;
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
            std::cerr << "Error saving file: " << eResultSave << std::endl;
            return -1;
        }

        std::cout << nameCStr << "obj Save game written to '" << filename << ".xml'\n";

        return 0;

    }


    template<typename ComponentTemplate>
    static auto getComponent(std::string name) {

        /*

        example of use

        TransformComponent* tempTestWrite = std::apply([](auto&&... args) {
        return new TransformComponent(args...);
        }, XMLtest::getComponent<TransformComponent>("Bob"));

        */

        std::string path = "Game Objects/" + SceneGraph::getInstance().cellFileName + "/" + name + ".xml";

        const char* id = path.c_str();

        XMLDocument doc;

        std::string componentType = static_cast<std::string>(typeid(ComponentTemplate).name()).substr(6);


        XMLError eResult = doc.LoadFile(id);
        if (eResult != XML_SUCCESS) {
            std::cerr << "Error loading file " << id << ": " << eResult << std::endl;
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
            auto args = std::make_tuple(nullptr,
                posArg,
                rotationArg,
                scaleArg);
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
        else if constexpr (std::is_same_v<ComponentTemplate, MeshComponent> || std::is_same_v<ComponentTemplate, MaterialComponent> || std::is_same_v<ComponentTemplate, ShaderComponent>) {

            AssetManager& assetMgr = AssetManager::getInstance();
            

            
                
            //return the asset reference in AssetManager named in element
            std::string assetName = component->FindAttribute("name")->Value();

            const char* assetNameCStr = assetName.c_str();

            auto args = assetName;
            return args;



        }

    }



    ///Simplify FindAttribute() implimentation for coding convenience
    static inline float GetAttrF(XMLElement* element, const char* name) {
        return element->FindAttribute(name)->FloatValue();
    }

    template<typename ComponentTemplate>
    static int writeUniqueComponent(std::string name, Component* toWrite) {
        std::string path = "Game Objects/" + SceneGraph::getInstance().cellFileName + "/" + name + ".xml";
        const char* id = path.c_str();
        XMLDocument doc;

        //try loading the file into doc
        XMLError eResult = doc.LoadFile(id);
        if (eResult != XML_SUCCESS) {
            std::cerr << "Error loading file " << id << ": " << eResult << std::endl;
            return eResult;
        }


        std::cout << "Found file to write: " << name << std::endl;


        //the basic format for any component
        XMLNode* cRoot = doc.RootElement();
        std::string componentType = static_cast<std::string>(typeid(ComponentTemplate).name()).substr(6);

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

            Quaternion rot = componentToWrite->GetQuaternion();

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
        else if constexpr (std::is_same_v<ComponentTemplate, CameraComponent>) {
        
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

        }
        else {

            std::cout << "ComponentWriteError: " << componentType << " is not a supported component type" << std::endl;
        }







        cRoot->InsertEndChild(componentElement);
        doc.Print();

        XMLError eResultSave = doc.SaveFile(id);

        if (eResultSave != XML_SUCCESS) {
            std::cerr << "Error saving file: " << eResultSave << std::endl;
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
    static int writeReferenceComponent(std::string name, Ref<Component> toWrite) {
        std::string path = "Game Objects/" + SceneGraph::getInstance().cellFileName + "/" + name + ".xml";
        const char* id = path.c_str();



        XMLDocument doc;

        //try loading the file into doc
        XMLError eResult = doc.LoadFile(id);
        if (eResult != XML_SUCCESS) {
            std::cerr << "Error loading file " << id << ": " << eResult << std::endl;
            return eResult;
        }



        std::cout << "Found file to write: " << name << std::endl;

        XMLNode* cRoot = doc.RootElement();

        std::string componentType = static_cast<std::string>(typeid(ComponentTemplate).name()).substr(6);


        XMLElement* componentElement = cRoot->FirstChildElement(componentType.c_str());
        if (componentElement) {
            std::cout << "Found component (Deleting): " << name << std::endl;

            componentElement->DeleteChildren();
        }
        std::cout << "Creating Element: " << componentType << std::endl;

        componentElement = doc.NewElement(componentType.c_str());


        AssetManager& assetMgr = AssetManager::getInstance();


        std::string assetStringName = assetMgr.getAssetName(toWrite);

        const char* assetName = assetStringName.c_str();

        //Add the cstr name as attribute
        componentElement->SetAttribute("name", assetName);




        cRoot->InsertEndChild(componentElement);
        doc.Print();

        XMLError eResultSave = doc.SaveFile(id);

        if (eResultSave != XML_SUCCESS) {
            std::cerr << "Error saving file: " << eResultSave << std::endl;
            return -1;
        }

        std::cout << "Save game written to '" << name << ".xml'\n";

        return 0;
    }

    /// <summary>
    /// Used to check if an actor has a specific type of component in their XML file
    /// </summary>
    /// <typeparam name="ComponentTemplate"></typeparam>
    /// <param name="name">name of the actor</param>
    /// <returns>true if the component type exists within the XML file</returns>
    template<typename ComponentTemplate>
    static bool hasComponent(std::string name) {
        std::string path = "Game Objects/" + SceneGraph::getInstance().cellFileName + "/" + name + ".xml";
        const char* id = path.c_str();

        XMLDocument doc;
        //try loading the file into doc
        XMLError eResult = doc.LoadFile(id);
        if (eResult != XML_SUCCESS) {
            std::cerr << "Error loading file " << id << ": " << eResult << std::endl;
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

    //int readDoc() {
    //    XMLDocument doc;

    //    // Root element
    //    XMLNode* pRoot = doc.NewElement("SaveGame");
    //    doc.InsertFirstChild(pRoot);

    //    // Player element with attributes
    //    XMLElement* pPlayer = doc.NewElement("Player");
    //    pPlayer->SetAttribute("name", "Hero");
    //    pPlayer->SetAttribute("level", 5);
    //    pPlayer->SetAttribute("health", 87.5);
    //    pRoot->InsertEndChild(pPlayer);

    //    // Inventory
    //    XMLElement* pInventory = doc.NewElement("Inventory");

    //    XMLElement* pItem1 = doc.NewElement("Item");
    //    pItem1->SetAttribute("name", "Sword");
    //    pItem1->SetAttribute("quantity", 1);
    //    pInventory->InsertEndChild(pItem1);

    //    XMLElement* pItem2 = doc.NewElement("Item");
    //    pItem2->SetAttribute("name", "Potion");
    //    pItem2->SetAttribute("quantity", 3);
    //    pInventory->InsertEndChild(pItem2);

    //    pPlayer->InsertEndChild(pInventory);

    //    // Save to file
    //    XMLError eResult = doc.SaveFile("Game Objects/savegame.xml");
    //    if (eResult != XML_SUCCESS) {
    //        std::cerr << "Error saving file: " << eResult << std::endl;
    //        return -1;
    //    }

    //    std::cout << "Save game written to 'savegame.xml'\n";
    //    return 0;
    //}
};

//Requires XMLManager to exist


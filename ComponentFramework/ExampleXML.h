#pragma once
#include <iostream>
#include <string>

#include "tinyxml2.h"
#include "Component.h"
template <typename T>
constexpr auto TypeName = static_cast<std::string>(typeid(T).name()).substr(6);

using namespace tinyxml2;

class XMLtest {
public:

    static int writeActor(std::string name) {
        XMLDocument doc;

        XMLNode* cRoot = doc.NewElement("Actor");
        doc.InsertFirstChild(cRoot);



        doc.SaveFile(("Game Objects/" + name + ".xml").c_str());
        return 0;
    }

    template<typename ComponentTemplate>
    static int writeComponent(std::string name, Component* toWrite) {
        std::string path = "Game Objects/" + name + ".xml";
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

        std::cout << "Save game written to '" << name <<".xml'\n";

        return 0;
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
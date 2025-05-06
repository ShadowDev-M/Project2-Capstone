#include "ExampleXML.h"

void XMLObjectFile::addAttributeRecursive(SceneGraph* sceneGraph, const XMLAttribute* attribute) {
        Ref<Actor> actorToAdd = std::make_shared<Actor>(nullptr, attribute->Value());


        std::string nameStr = (attribute->Value());       

       
        std::tuple tupleArgs = XMLObjectFile::getComponent<TransformComponent>(nameStr);

        actorToAdd->AddComponent<TransformComponent>(

            std::apply([](auto&&... args) {
                return new TransformComponent(args...);
                }, tupleArgs)

        );
        sceneGraph->AddActor(std::make_shared<Actor>(nullptr, attribute->Value()));

        if (attribute->Next()) addAttributeRecursive(sceneGraph, attribute->Next());   
    }

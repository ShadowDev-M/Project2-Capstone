#include "ExampleXML.h"

void XMLObjectFile::addAttributeRecursive(SceneGraph* sceneGraph, const XMLAttribute* attribute) {
       

        Ref<Actor> actorToAdd = sceneGraph->GetActor(attribute->Value());
        if (actorToAdd == nullptr) {
            actorToAdd = std::make_shared<Actor>(nullptr, attribute->Value());
        }


        std::string nameStr = (attribute->Value());       

       
        std::tuple tupleArgs = XMLObjectFile::getComponent<TransformComponent>(nameStr);

        //If the transform exists, the transform should be overwritten vs created
        if (actorToAdd->GetComponent<TransformComponent>()) {

            Ref<TransformComponent> oldTransform = actorToAdd->GetComponent<TransformComponent>();

            //restore pos 
            Vec3 rePos = std::get<1>(tupleArgs);

            oldTransform->SetPos(rePos.x, rePos.y, rePos.z);

            //restore quat 
            Quaternion reQuat = std::get<2>(tupleArgs);

            oldTransform->SetOrientation(reQuat);
            
            //restore scale 
            Vec3 reScale = std::get<3>(tupleArgs);

            oldTransform->SetTransform(reScale);
        }
        else {
            //create new transform
            actorToAdd->AddComponent<TransformComponent>(

                std::apply([](auto&&... args) {
                    return new TransformComponent(args...);
                    }, tupleArgs)

            );
        }
        sceneGraph->AddActor(std::make_shared<Actor>(nullptr, attribute->Value()));

        if (attribute->Next()) addAttributeRecursive(sceneGraph, attribute->Next());   
    }

void SceneGraph::SaveFile(std::string name) {
    XMLObjectFile::writeCellFile(name);

    for (auto& obj : Actors) {
        
        XMLObjectFile::writeActor(obj.first);
        XMLObjectFile::writeComponent<TransformComponent>(obj.first, obj.second->GetComponent<TransformComponent>().get());
        obj.second->GetComponent<TransformComponent>()->GetPosition().print();
        XMLObjectFile::writeActorToCell(name, obj.first, true);
    }
}

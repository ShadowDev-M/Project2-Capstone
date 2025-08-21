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

void SceneGraph::SaveFile(std::string name) const {
    XMLObjectFile::writeCellFile(name);

    for (auto& obj : Actors) {
        
        XMLObjectFile::writeActor(obj.first);

        XMLObjectFile::writeUniqueComponent<TransformComponent>(obj.first, obj.second->GetComponent<TransformComponent>().get());

        AssetManager& assetMgr = AssetManager::getInstance();

        std::cout << assetMgr.getAssetName(obj.second->GetComponent<MeshComponent>()) << std::endl;

        if (obj.second->GetComponent<MeshComponent>())
            XMLObjectFile::writeReferenceComponent<MeshComponent>(obj.first, obj.second->GetComponent<MeshComponent>());
        
        if (obj.second->GetComponent<MaterialComponent>())
            XMLObjectFile::writeReferenceComponent<MaterialComponent>(obj.first, obj.second->GetComponent<MaterialComponent>());

        if (obj.second->GetComponent<ShaderComponent>())
            XMLObjectFile::writeReferenceComponent<ShaderComponent>(obj.first, obj.second->GetComponent<ShaderComponent>());


        // [key.first/second] accesses the vector for the given key, if it doesn't exist it creates it

       /* XMLObjectFile::writeComponent<MeshComponent>(obj.first, obj.second->GetComponent<MeshComponent>().get());

        XMLObjectFile::writeComponent<MaterialComponent>(obj.first, obj.second->GetComponent<MaterialComponent>().get());

        XMLObjectFile::writeComponent<ShaderComponent>(obj.first, obj.second->GetComponent<ShaderComponent>().get());*/


        obj.second->GetComponent<TransformComponent>()->GetPosition().print();
        XMLObjectFile::writeActorToCell(name, obj.first, true);
    }


    for (auto& component_ : AssetManager::getInstance().GetAllAssetKeyPair()) {

        XMLObjectFile::writeComponentToCell(name, component_.first, component_.second, true);

    }


}

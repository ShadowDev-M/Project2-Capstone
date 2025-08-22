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




void XMLObjectFile::runCreateActorsOfElementChildren(SceneGraph* sceneGraph, XMLElement* actorElement, XMLElement* rootElement) {

    //During the first run of the function, (if rootElement is empty) the root would be set to the first actorElement which should be the root.
    //Future runs will refer back to this value from the first run
    if (rootElement == nullptr) {
        rootElement = actorElement;
    }

    //create all children of actorElement and have them create their children
    for (XMLElement* it = actorElement->FirstChildElement(); it != nullptr; it = it->NextSiblingElement()) {

        //Create actor
        

        std::string itName = it->Name();
        const char* itNameCstr = itName.c_str();

        //Camera is excluded until camera is reworked 
        if (itName != "camera") {

            std::string actorName = actorElement->Name();

            // The root element shouldn't be included in this, as its create actors of element's children NOT INCLUDING THE ELEMENT
            // After all, you don't want to accidentally create an actor called "Actors" (The container in the save file for the actors)
            if (actorElement == rootElement) {
                //If the element is the same as the root therefore ignore the parent as to not conflict with any actors called "Actors" 
                sceneGraph->LoadActor(itNameCstr);

            }
            else {
                //Actor must therefore have parent of actorElement's name
                sceneGraph->LoadActor(itNameCstr, sceneGraph->GetActor(actorName));

            }

            createActorFromElement(sceneGraph, it);

        }

        //after actor is created, create its children
        runCreateActorsOfElementChildren(sceneGraph, it, rootElement);

    }

}


void XMLObjectFile::createActorFromElement(SceneGraph* sceneGraph, XMLElement* actorElement) {



    //if actor exists in scenegraph just use that rather than make a new one
    Ref<Actor> actorToAdd = sceneGraph->GetActor(actorElement->Name());

    //make new actor if not existing beforehand
    if (actorToAdd == nullptr) {
        actorToAdd = std::make_shared<Actor>(nullptr, actorElement->Name());
    }

    std::string nameStr = (actorElement->Name());

    //arguements for transform component
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

    //Add to scenegraph
    sceneGraph->AddActor(std::make_shared<Actor>(nullptr, actorElement->Name()));

    //        if (attribute->Next()) addAttributeRecursive(sceneGraph, attribute->Next());   
}


//
XMLElement* XMLObjectFile::writeActorRecursive(Actor* actor_, XMLElement* root_) {

    XMLDocument* doc = root_->GetDocument();

    //element to attach object to    
    XMLElement* parentElement;

    //Get parent of actor
    Actor* parentActor = actor_->getParentActor();


    //if there's a parent of the actor, get/create recursively the element
    //if no parent, the 'parent' is the root_

    if (parentActor == actor_) {
        parentActor = nullptr; // break self-loop
    }

    if (parentActor) {

        parentElement = writeActorRecursive(parentActor, root_);

    }
    else {

        parentElement = root_;

    }

    XMLElement* actorObjElement;

    std::string name = actor_->getActorName();

    std::cout << name << " recursion writing" << std::endl;

    //check element already exists
    if (actorObjElement = parentElement->FirstChildElement(name.c_str())) {

        //actor already exists, return the element thats already there
        return actorObjElement;


    }
    //no element exists if it didn't return

    //make a new element and make it a child of the in-code parent (or root if none)
    actorObjElement = doc->NewElement(name.c_str());

    // add flags as attributes here if needed
    {


    }


    parentElement->InsertEndChild(actorObjElement);

    return actorObjElement;

};


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
        XMLObjectFile::writeActorToCell(name, obj.second, true);
    }


    for (auto& component_ : AssetManager::getInstance().GetAllAssetKeyPair()) {

        XMLObjectFile::writeComponentToCell(name, component_.first, component_.second, true);

    }


}

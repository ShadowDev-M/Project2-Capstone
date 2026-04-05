#include "pch.h"
#include "XMLManager.h"
#include "ScriptComponent.h"
#include "PhysicsSystem.h"
#include "CollisionSystem.h"
#include "LightingSystem.h"
#include <memory>

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

void XMLObjectFile::WriteMatManifest(const fs::path& outputPath, const std::string& diffuseRelative, const std::string& specularRelative, const std::string& normalRelative)
{
    XMLDocument doc;
    auto* root = doc.NewElement("Material");
    doc.InsertFirstChild(root);

    // helper lambda to insert element
    auto addEl = [&](const char* tag, const std::string& val) {
        if (val.empty()) return;
        auto* el = doc.NewElement(tag);
        el->SetText(val.c_str());
        root->InsertEndChild(el);
        };

    addEl("Diffuse", diffuseRelative);
    addEl("Specular", specularRelative);
    addEl("Normal", normalRelative);

    doc.SaveFile(outputPath.string().c_str());
}

void XMLObjectFile::WriteShaderManifest(const fs::path& outputPath, const std::string& vertRel, const std::string& fragRel, const std::string& tessCtrlRel, const std::string& tessEvalRel, const std::string& geomRel)
{
    XMLDocument doc;
    auto* root = doc.NewElement("Shader");
    doc.InsertFirstChild(root);

    // helper lambda to insert element
    auto addEl = [&](const char* tag, const std::string& val) {
        if (val.empty()) return;
        auto* el = doc.NewElement(tag);
        el->SetText(val.c_str());
        root->InsertEndChild(el);
        };

    addEl("Vertex", vertRel);
    addEl("Fragment", fragRel);
    addEl("TessControl", tessCtrlRel);
    addEl("TessEval", tessEvalRel);
    addEl("Geometry", geomRel);

    doc.SaveFile(outputPath.string().c_str());
}

bool XMLObjectFile::WritePrefab(const std::string& actorName, const fs::path& outputPath)
{
    Ref<Actor> actor = SceneGraph::getInstance().GetActor(actorName);
    if (!actor) return false;

    XMLDocument doc;
    auto* root = doc.NewElement("Prefab");
    root->SetAttribute("name", actorName.c_str());
    doc.InsertFirstChild(root);
    WritePrefabNode(doc, root, actorName);
    return doc.SaveFile(outputPath.string().c_str()) == XML_SUCCESS;
}

Ref<Actor> XMLObjectFile::ReadPrefab(const fs::path& prefabPath, std::vector<Ref<Actor>>& outActors)
{
    XMLDocument doc;
    if (doc.LoadFile(prefabPath.string().c_str()) != XML_SUCCESS) return nullptr;
    auto* root = doc.FirstChildElement("Prefab");
    if (!root) return nullptr;
    return ReadPrefabNode(root, nullptr, outActors);
}

void XMLObjectFile::WritePrefabNode(XMLDocument& doc, XMLElement* parentEl, const std::string& actorName)
{
    // basically like when saving an actor, but everything gets saved even childern and their components
    // everything for the selected actor gets written down and copied into the .prefab file

    Ref<Actor> actor = SceneGraph::getInstance().GetActor(actorName);
    if (!actor) return;

    AssetManager& am = AssetManager::getInstance();

    // tag
    auto* tagEl = doc.NewElement("Tag");
    tagEl->SetAttribute("value", actor->getTag().c_str());
    parentEl->InsertEndChild(tagEl);

    // transform
    if (auto tc = actor->GetComponent<TransformComponent>()) {
        auto* tcEl = doc.NewElement("TransformComponent");
        Vec3 pos = tc->GetPosition();
        Quaternion q = tc->GetOrientation();
        Vec3 sc = tc->GetScale();

        auto* posEl = doc.NewElement("position");
        posEl->SetAttribute("x", pos.x); posEl->SetAttribute("y", pos.y); posEl->SetAttribute("z", pos.z);
        tcEl->InsertEndChild(posEl);

        auto* rotEl = doc.NewElement("rotation");
        rotEl->SetAttribute("w", q.w); rotEl->SetAttribute("x", q.ijk.x);
        rotEl->SetAttribute("y", q.ijk.y); rotEl->SetAttribute("z", q.ijk.z);
        tcEl->InsertEndChild(rotEl);

        auto* scEl = doc.NewElement("scale");
        scEl->SetAttribute("x", sc.x); scEl->SetAttribute("y", sc.y); scEl->SetAttribute("z", sc.z);
        tcEl->InsertEndChild(scEl);

        parentEl->InsertEndChild(tcEl);
    }

    // physics
    if (auto pc = actor->GetComponent<PhysicsComponent>()) {
        auto* el = doc.NewElement("PhysicsComponent");
        const auto& con = pc->getConstraints();

        auto addState = [&](const char* tag, const char* attr, int v) {
            auto* e = doc.NewElement(tag); e->SetAttribute(attr, v); el->InsertEndChild(e);
            };
        auto addFloat = [&](const char* tag, const char* attr, float v) {
            auto* e = doc.NewElement(tag); e->SetAttribute(attr, v); el->InsertEndChild(e);
            };
        auto addBool = [&](const char* tag, const char* attr, bool v) {
            auto* e = doc.NewElement(tag); e->SetAttribute(attr, v ? 1 : 0); el->InsertEndChild(e);
            };

        addState("PhysicsState", "state", (int)pc->getState());

        auto* cEl = doc.NewElement("Constraints");
        cEl->SetAttribute("freezePosX", con.freezePosX); cEl->SetAttribute("freezePosY", con.freezePosY);
        cEl->SetAttribute("freezePosZ", con.freezePosZ); cEl->SetAttribute("freezeRotX", con.freezeRotX);
        cEl->SetAttribute("freezeRotY", con.freezeRotY); cEl->SetAttribute("freezeRotZ", con.freezeRotZ);
        el->InsertEndChild(cEl);

        addFloat("mass", "value", pc->getMass());
        addBool("UsingGravity", "isUsing", pc->getUseGravity());
        addFloat("drag", "value", pc->getDrag());
        addFloat("angularDrag", "value", pc->getAngularDrag());
        addFloat("friction", "value", pc->getFriction());
        addFloat("restitution", "value", pc->getRestitution());

        parentEl->InsertEndChild(el);
    }

    // collision
    if (auto cc = actor->GetComponent<CollisionComponent>()) {
        auto* el = doc.NewElement("CollisionComponent");

        auto addStateEl = [&](const char* tag, const char* attr, int v) {
            auto* e = doc.NewElement(tag); e->SetAttribute(attr, v); el->InsertEndChild(e);
            };
        addStateEl("CollisionState", "state", (int)cc->getState());
        addStateEl("CollisionType", "type", (int)cc->getType());
        addStateEl("isTrigger", "trigger", cc->getIsTrigger() ? 1 : 0);

        auto* rEl = doc.NewElement("radius"); rEl->SetAttribute("value", cc->getRadius()); el->InsertEndChild(rEl);

        auto addVec3El = [&](const char* tag, Vec3 v) {
            auto* e = doc.NewElement(tag);
            e->SetAttribute("x", v.x); e->SetAttribute("y", v.y); e->SetAttribute("z", v.z);
            el->InsertEndChild(e);
            };
        addVec3El("centre", cc->getCentre());
        addVec3El("centrePosA", cc->getCentrePosA());
        addVec3El("centrePosB", cc->getCentrePosB());
        addVec3El("halfExtents", cc->getHalfExtents());

        auto* oriEl = doc.NewElement("orientation");
        Quaternion ori = cc->getOrientation();
        oriEl->SetAttribute("w", ori.w); oriEl->SetAttribute("x", ori.ijk.x);
        oriEl->SetAttribute("y", ori.ijk.y); oriEl->SetAttribute("z", ori.ijk.z);
        el->InsertEndChild(oriEl);

        parentEl->InsertEndChild(el);
    }

    // animator
    if (auto ac = actor->GetComponent<AnimatorComponent>()) {
        auto* el = doc.NewElement("AnimatorComponent");
        auto* stEl = doc.NewElement("startTime"); stEl->SetAttribute("time", ac->getStartTime()); el->InsertEndChild(stEl);
        auto* spEl = doc.NewElement("speedMult"); spEl->SetAttribute("mult", ac->getSpeedMult()); el->InsertEndChild(spEl);
        auto* loEl = doc.NewElement("looping"); loEl->SetAttribute("state", ac->getLoopingState()); el->InsertEndChild(loEl);
        auto* anEl = doc.NewElement("animName"); anEl->SetAttribute("name", ac->getAnimName().c_str()); el->InsertEndChild(anEl);
        parentEl->InsertEndChild(el);
    }

    // shadow
    if (auto shadowSettings = actor->GetComponent<ShadowSettings>()) {
        auto* el = doc.NewElement("ShadowSettings");
        auto* cs = doc.NewElement("castShadow"); cs->SetAttribute("state", shadowSettings->getCastShadow() ? 1 : 0);
        el->InsertEndChild(cs);
        parentEl->InsertEndChild(el);
    }

    // tiling
    if (auto ts = actor->GetComponent<TilingSettings>()) {
        auto* el = doc.NewElement("TilingSettings");
        auto* it = doc.NewElement("isTiled"); it->SetAttribute("state", ts->getIsTiled() ? 1 : 0); el->InsertEndChild(it);
        auto* sc = doc.NewElement("tileScale"); sc->SetAttribute("x", ts->getTileScale().x); sc->SetAttribute("y", ts->getTileScale().y); el->InsertEndChild(sc);
        auto* ofs = doc.NewElement("tileOffset"); ofs->SetAttribute("x", ts->getTileOffset().x); ofs->SetAttribute("y", ts->getTileOffset().y); el->InsertEndChild(ofs);
        parentEl->InsertEndChild(el);
    }

    // light
    if (auto lc = actor->GetComponent<LightComponent>()) {
        auto* el = doc.NewElement("LightComponent");

        auto* diffEl = doc.NewElement("diffuse");
        diffEl->SetAttribute("x", lc->getDiff().x); diffEl->SetAttribute("y", lc->getDiff().y);
        diffEl->SetAttribute("z", lc->getDiff().z); diffEl->SetAttribute("w", lc->getDiff().w);
        el->InsertEndChild(diffEl);

        auto* specEl = doc.NewElement("specular");
        specEl->SetAttribute("x", lc->getSpec().x); specEl->SetAttribute("y", lc->getSpec().y);
        specEl->SetAttribute("z", lc->getSpec().z); specEl->SetAttribute("w", lc->getSpec().w);
        el->InsertEndChild(specEl);

        auto* intEl = doc.NewElement("intensity"); intEl->SetAttribute("magnitude", lc->getIntensity()); el->InsertEndChild(intEl);
        auto* typeEl = doc.NewElement("LightType"); typeEl->SetAttribute("type", (int)lc->getType()); el->InsertEndChild(typeEl);
        auto* stEl = doc.NewElement("shadowType"); stEl->SetAttribute("type", (int)lc->getShadowType()); el->InsertEndChild(stEl);
        auto* snEl = doc.NewElement("shadowNear"); snEl->SetAttribute("value", lc->getShadowNear()); el->InsertEndChild(snEl);
        auto* sfEl = doc.NewElement("shadowFar"); sfEl->SetAttribute("value", lc->getShadowFar()); el->InsertEndChild(sfEl);
        auto* srEl = doc.NewElement("shadowResolution"); srEl->SetAttribute("value", lc->getShadowResolution()); el->InsertEndChild(srEl);
        auto* soEl = doc.NewElement("shadowOrthoSize"); soEl->SetAttribute("value", lc->getShadowOrthoSize()); el->InsertEndChild(soEl);

        parentEl->InsertEndChild(el);
    }

    // camera
    if (auto cam = actor->GetComponent<CameraComponent>()) {
        auto* el = doc.NewElement("CameraComponent");
        auto* typeEl = doc.NewElement("type"); typeEl->SetAttribute("value", (int)cam->getType()); el->InsertEndChild(typeEl);
        auto* fovEl = doc.NewElement("fov"); fovEl->SetAttribute("value", cam->getFOV()); el->InsertEndChild(fovEl);
        auto* nearEl = doc.NewElement("near"); nearEl->SetAttribute("value", cam->getNearClipPlane()); el->InsertEndChild(nearEl);
        auto* farEl = doc.NewElement("far"); farEl->SetAttribute("value", cam->getFarClipPlane()); el->InsertEndChild(farEl);
        auto* orthoEl = doc.NewElement("orthoSize"); orthoEl->SetAttribute("value", cam->getOrthoSize()); el->InsertEndChild(orthoEl);
        parentEl->InsertEndChild(el);
    }

    // ref components
    auto writeRef = [&](const char* tag, Ref<Component> comp) {
        if (!comp) return;
        std::string assetName = am.GetAssetName(comp);
        if (assetName.empty()) return;
        auto* e = doc.NewElement(tag);
        e->SetAttribute("name", assetName.c_str());
        parentEl->InsertEndChild(e);
        };

    writeRef("MeshComponent", actor->GetComponent<MeshComponent>());
    writeRef("MaterialComponent", actor->GetComponent<MaterialComponent>());
    writeRef("ShaderComponent", actor->GetComponent<ShaderComponent>());
    for (auto& sc : actor->GetAllComponent<ScriptComponent>()) {
        writeRef("ScriptComponent", sc->getBaseAsset());
    }

    // recursive child lookup
    bool hasChildren = false;
    XMLElement* childrenEl = nullptr;
    for (auto& [id, child] : SceneGraph::getInstance().getAllActors()) {
        if (child->getParentActor() == actor.get()) {
            if (!hasChildren) {
                childrenEl = doc.NewElement("Children");
                parentEl->InsertEndChild(childrenEl);
                hasChildren = true;
            }
            auto* childEl = doc.NewElement("Child");
            childEl->SetAttribute("name", child->getActorName().c_str());
            childrenEl->InsertEndChild(childEl);
            WritePrefabNode(doc, childEl, child->getActorName());
        }
    }
}

Ref<Actor> XMLObjectFile::ReadPrefabNode(XMLElement* nodeEl, Actor* parent, std::vector<Ref<Actor>>& outActors)
{
    const char* nameAttr = nodeEl->Attribute("name");
    if (!nameAttr) return nullptr;

    std::string name = nameAttr;
    auto actor = std::make_shared<Actor>(parent, name);
    AssetManager& am = AssetManager::getInstance();

    // tag
    if (auto* el = nodeEl->FirstChildElement("Tag")) {
        const char* v = el->Attribute("value");
        actor->setTag(v ? v : "Untagged");
    }

    // transform
    if (auto* tcEl = nodeEl->FirstChildElement("TransformComponent")) {
        Vec3 pos(0, 0, 0), sc(1, 1, 1);
        Quaternion ori(1, Vec3(0, 0, 0));
        if (auto* e = tcEl->FirstChildElement("position")) {
            e->QueryFloatAttribute("x", &pos.x); 
            e->QueryFloatAttribute("y", &pos.y);
            e->QueryFloatAttribute("z", &pos.z);
        }
        if (auto* e = tcEl->FirstChildElement("rotation")) {
            e->QueryFloatAttribute("w", &ori.w); e->QueryFloatAttribute("x", &ori.ijk.x);
            e->QueryFloatAttribute("y", &ori.ijk.y); e->QueryFloatAttribute("z", &ori.ijk.z);
        }
        if (auto* e = tcEl->FirstChildElement("scale")) {
            e->QueryFloatAttribute("x", &sc.x); 
            e->QueryFloatAttribute("y", &sc.y);
            e->QueryFloatAttribute("z", &sc.z);
        }
        actor->AddComponent<TransformComponent>(std::make_shared<TransformComponent>(actor.get(), pos, ori, sc));
    }

    // mesh
    if (auto* el = nodeEl->FirstChildElement("MeshComponent")) {
        const char* n = el->Attribute("name");
        if (n) {
            if (auto m = am.GetAsset<MeshComponent>(n)) {
                actor->ReplaceComponent(m);
            }
        }
    }

    // material
    if (auto* el = nodeEl->FirstChildElement("MaterialComponent")) {
        const char* n = el->Attribute("name");
        if (n) {
            if (auto m = am.GetAsset<MaterialComponent>(n)) {
                actor->ReplaceComponent(m);
            }
        }
    }

    // shader
    if (auto* el = nodeEl->FirstChildElement("ShaderComponent")) {
        const char* n = el->Attribute("name");
        if (n) if (auto s = am.GetAsset<ShaderComponent>(n)) actor->ReplaceComponent(s);
    }

    // shadow settings
    if (auto* el = nodeEl->FirstChildElement("ShadowSettings")) {
        int cast = 1;
        if (auto* cs = el->FirstChildElement("castShadow")) cs->QueryIntAttribute("state", &cast);
        if (!actor->GetComponent<ShadowSettings>()) {
            actor->AddComponent<ShadowSettings>(std::make_shared<ShadowSettings>(actor.get(), cast != 0));
        }
    }
    if (actor->GetComponent<MeshComponent>() && !actor->GetComponent<ShadowSettings>()) {
        actor->AddComponent<ShadowSettings>(std::make_shared<ShadowSettings>(actor.get(), true));
    }

    // tilingSettings
    if (auto* el = nodeEl->FirstChildElement("TilingSettings")) {
        int tiled = 0; float sx = 1, sy = 1, ox = 0, oy = 0;
        if (auto* e = el->FirstChildElement("isTiled")) e->QueryIntAttribute("state", &tiled);
        if (auto* e = el->FirstChildElement("tileScale")) {
            e->QueryFloatAttribute("x", &sx); e->QueryFloatAttribute("y", &sy);
        }
        if (auto* e = el->FirstChildElement("tileOffset")) {
            e->QueryFloatAttribute("x", &ox); e->QueryFloatAttribute("y", &oy);
        }
        if (!actor->GetComponent<TilingSettings>()) {
            actor->AddComponent<TilingSettings>(std::make_shared<TilingSettings>(actor.get(), tiled != 0, Vec2(sx, sy), Vec2(ox, oy)));
        }
    }
    if (actor->GetComponent<MaterialComponent>() && !actor->GetComponent<TilingSettings>()) {
        actor->AddComponent<TilingSettings>(std::make_shared<TilingSettings>(actor.get(), false));
    }

    // physics
    if (auto* el = nodeEl->FirstChildElement("PhysicsComponent")) {
        int state = 0; float mass = 1, drag = 0, angDrag = 0.05f, friction = 0.5f, rest = 0; int useGrav = 1;
        PhysicsConstraints con;
        if (auto* e = el->FirstChildElement("PhysicsState")) e->QueryIntAttribute("state", &state);
        if (auto* e = el->FirstChildElement("mass")) e->QueryFloatAttribute("value", &mass);
        if (auto* e = el->FirstChildElement("UsingGravity")) e->QueryIntAttribute("isUsing", &useGrav);
        if (auto* e = el->FirstChildElement("drag")) e->QueryFloatAttribute("value", &drag);
        if (auto* e = el->FirstChildElement("angularDrag")) e->QueryFloatAttribute("value", &angDrag);
        if (auto* e = el->FirstChildElement("friction")) e->QueryFloatAttribute("value", &friction);
        if (auto* e = el->FirstChildElement("restitution")) e->QueryFloatAttribute("value", &rest);
        if (auto* e = el->FirstChildElement("Constraints")) {
            e->QueryBoolAttribute("freezePosX", &con.freezePosX);
            e->QueryBoolAttribute("freezePosY", &con.freezePosY);
            e->QueryBoolAttribute("freezePosZ", &con.freezePosZ);
            e->QueryBoolAttribute("freezeRotX", &con.freezeRotX);
            e->QueryBoolAttribute("freezeRotY", &con.freezeRotY);
            e->QueryBoolAttribute("freezeRotZ", &con.freezeRotZ);
        }
        auto pc = std::make_shared<PhysicsComponent>(actor.get(), (PhysicsState)state, con, mass, useGrav != 0, drag, angDrag, friction, rest);
        actor->AddComponent(pc);
        PhysicsSystem::getInstance().AddActor(actor);
    }

    // collision
    if (auto* el = nodeEl->FirstChildElement("CollisionComponent")) {
        int state = 0, type = 0, trig = 0; float r = 1;
        Vec3 ctr, capA(0, 1, 0), capB(0, -1, 0), half(1, 1, 1);
        Quaternion ori(1, Vec3(0, 0, 0));
        if (auto* e = el->FirstChildElement("CollisionState")) e->QueryIntAttribute("state", &state);
        if (auto* e = el->FirstChildElement("CollisionType")) e->QueryIntAttribute("type", &type);
        if (auto* e = el->FirstChildElement("isTrigger")) e->QueryIntAttribute("trigger", &trig);
        if (auto* e = el->FirstChildElement("radius")) e->QueryFloatAttribute("value", &r);
        auto rv = [](XMLElement* e, Vec3& v) {
            if (!e) return;
            e->QueryFloatAttribute("x", &v.x); 
            e->QueryFloatAttribute("y", &v.y);
            e->QueryFloatAttribute("z", &v.z);
            };
        rv(el->FirstChildElement("centre"), ctr);
        rv(el->FirstChildElement("centrePosA"), capA);
        rv(el->FirstChildElement("centrePosB"), capB);
        rv(el->FirstChildElement("halfExtents"), half);
        if (auto* e = el->FirstChildElement("orientation")) {
            e->QueryFloatAttribute("w", &ori.w); e->QueryFloatAttribute("x", &ori.ijk.x);
            e->QueryFloatAttribute("y", &ori.ijk.y); e->QueryFloatAttribute("z", &ori.ijk.z);
        }
        auto cc = std::make_shared<CollisionComponent>(actor.get(), (ColliderState)state, (ColliderType)type, trig != 0, r, ctr, capA, capB, half, ori);
        actor->AddComponent(cc);
        CollisionSystem::getInstance().AddActor(actor);
    }

    // animation
    if (auto* el = nodeEl->FirstChildElement("AnimatorComponent")) {
        float start = 0, speed = 1; bool loop = false; const char* anim = nullptr;
        if (auto* e = el->FirstChildElement("startTime")) e->QueryFloatAttribute("time", &start);
        if (auto* e = el->FirstChildElement("speedMult")) e->QueryFloatAttribute("mult", &speed);
        if (auto* e = el->FirstChildElement("looping")) {
            int b = 0; e->QueryIntAttribute("state", &b); loop = b != 0;
        }
        if (auto* e = el->FirstChildElement("animName")) anim = e->Attribute("name"); {
            actor->AddComponent<AnimatorComponent>(std::make_shared<AnimatorComponent>(actor.get(), start, speed, loop, anim ? std::string(anim) : ""));
        }
    }

    // light
    if (auto* el = nodeEl->FirstChildElement("LightComponent")) {
        Vec4 diff(0.5f, 0.5f, 0.5f, 1), spec(1, 1, 1, 1);
        float intensity = 1, shadowNear = 0.1f, shadowFar = 200, shadowOrtho = 60;
        int lightType = 0, shadowType = 0, shadowRes = 1024;
        if (auto* e = el->FirstChildElement("diffuse")) {
            e->QueryFloatAttribute("x", &diff.x); e->QueryFloatAttribute("y", &diff.y);
            e->QueryFloatAttribute("z", &diff.z); e->QueryFloatAttribute("w", &diff.w);
        }
        if (auto* e = el->FirstChildElement("specular")) {
            e->QueryFloatAttribute("x", &spec.x); e->QueryFloatAttribute("y", &spec.y);
            e->QueryFloatAttribute("z", &spec.z); e->QueryFloatAttribute("w", &spec.w);
        }
        if (auto* e = el->FirstChildElement("intensity")) e->QueryFloatAttribute("magnitude", &intensity);
        if (auto* e = el->FirstChildElement("LightType")) e->QueryIntAttribute("type", &lightType);
        if (auto* e = el->FirstChildElement("shadowType")) e->QueryIntAttribute("type", &shadowType);
        if (auto* e = el->FirstChildElement("shadowNear")) e->QueryFloatAttribute("value", &shadowNear);
        if (auto* e = el->FirstChildElement("shadowFar")) e->QueryFloatAttribute("value", &shadowFar);
        if (auto* e = el->FirstChildElement("shadowResolution")) e->QueryIntAttribute("value", &shadowRes);
        if (auto* e = el->FirstChildElement("shadowOrthoSize")) e->QueryFloatAttribute("value", &shadowOrtho);
        auto lc = std::make_shared<LightComponent>(actor.get(), (LightType)lightType, spec, diff, intensity);
        lc->setShadowType((ShadowType)shadowType);
        lc->setShadowNear(shadowNear); lc->setShadowFar(shadowFar);
        lc->setShadowResolution(shadowRes); lc->setShadowOrthoSize(shadowOrtho);
        actor->AddComponent(lc);
        LightingSystem::getInstance().AddActor(actor);
    }

    // camera
    if (auto* el = nodeEl->FirstChildElement("CameraComponent")) {
        int type = 0; float fov = 60, near = 0.03f, far = 1000, ortho = 5;
        if (auto* e = el->FirstChildElement("type")) e->QueryIntAttribute("value", &type);
        if (auto* e = el->FirstChildElement("fov")) e->QueryFloatAttribute("value", &fov);
        if (auto* e = el->FirstChildElement("near")) e->QueryFloatAttribute("value", &near);
        if (auto* e = el->FirstChildElement("far")) e->QueryFloatAttribute("value", &far);
        if (auto* e = el->FirstChildElement("orthoSize")) e->QueryFloatAttribute("value", &ortho);
        auto cam = std::make_shared<CameraComponent>(actor.get(), (ProjectionType)type, fov, near, far, ortho);
        actor->AddComponent(cam);
        if (actor->getTag() == "MainCamera") SceneGraph::getInstance().SetMainCamera(actor);
    }

    // scripts
    for (auto* el = nodeEl->FirstChildElement("ScriptComponent");
        el; el = el->NextSiblingElement("ScriptComponent")) {
        const char* n = el->Attribute("name");
        if (n) {
            if (auto sa = am.GetAsset<ScriptAbstract>(n)) {
                actor->AddComponent<ScriptComponent>(std::make_shared<ScriptComponent>(actor.get(), sa));
            }
        }
    }

    actor->OnCreate();

    outActors.push_back(actor);

    if (auto* childrenEl = nodeEl->FirstChildElement("Children")) {
        for (auto* childEl = childrenEl->FirstChildElement("Child"); childEl; childEl = childEl->NextSiblingElement("Child")) {
            ReadPrefabNode(childEl, actor.get(), outActors);
        }
    }

    return actor;
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
    std::tuple tupleArgs = std::tuple_cat(std::make_tuple(actorToAdd.get()), XMLObjectFile::getComponent<TransformComponent>(nameStr));

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

        oldTransform->SetScale(reScale);
    }
    else {
        //create new transform
        actorToAdd->AddComponent<TransformComponent>(

            std::apply([](auto&&... args) {
                RECORD return std::make_shared<TransformComponent>(args...);
                }, tupleArgs)

        );
    }

    //Add to scenegraph
    sceneGraph->AddActor(std::make_shared<Actor>(nullptr, actorElement->Name()));  
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




bool SceneGraph::RenameActor(const std::string& oldName_, const std::string& newName_)
{
    if (oldName_ == newName_) return true;
    if (newName_.empty()) {
        Debug::Warning("Cannot rename actor to empty name", __FILE__, __LINE__);
        return false;
    }

    // check if new name is already taken
    auto nameIt = ActorNameToId.find(newName_);
    if (nameIt != ActorNameToId.end()) {
        Debug::Warning("An actor named: " + newName_ + " already exists", __FILE__, __LINE__);
        return false;
    }

    // find the actor by old name
    auto oldNameIt = ActorNameToId.find(oldName_);
    if (oldNameIt == ActorNameToId.end()) {
        Debug::Error("Cannot find actor named: " + oldName_, __FILE__, __LINE__);
        return false;
    }

    uint32_t actorId = oldNameIt->second;
    Ref<Actor> actor = Actors[actorId];

    // update the actor's internal name
    actor->setActorName(newName_);

    // update the name lookup map
    UpdateActorNameMap(actorId, oldName_, newName_);

    return true;
}

void SceneGraph::SaveFile(std::string name) const {
    XMLObjectFile::writeSceneFile(name);

    XMLObjectFile::writeSceneTags(name, allTags);

    for (auto& obj : ActorNameToId) {

        XMLObjectFile::writeActor(obj.first);

        XMLObjectFile::writeActorTag(obj.first, GetActor(obj.first)->getTag());

        XMLObjectFile::writeUniqueComponent<TransformComponent>(obj.first, GetActor(obj.first)->GetComponent<TransformComponent>().get());

        if (GetActor(obj.first)->GetComponent<PhysicsComponent>())
            XMLObjectFile::writeUniqueComponent<PhysicsComponent>(obj.first, GetActor(obj.first)->GetComponent<PhysicsComponent>().get());

        if (GetActor(obj.first)->GetComponent<CollisionComponent>())
            XMLObjectFile::writeUniqueComponent<CollisionComponent>(obj.first, GetActor(obj.first)->GetComponent<CollisionComponent>().get());

        //write camera as unique as there's no camera 'asset'
        if (GetActor(obj.first)->GetComponent<CameraComponent>())
            XMLObjectFile::writeUniqueComponent<CameraComponent>(obj.first, GetActor(obj.first)->GetComponent<CameraComponent>().get());

        if (GetActor(obj.first)->GetComponent<LightComponent>())
            XMLObjectFile::writeUniqueComponent<LightComponent>(obj.first, GetActor(obj.first)->GetComponent<LightComponent>().get());

        if (GetActor(obj.first)->GetComponent<AnimatorComponent>())
            XMLObjectFile::writeUniqueComponent<AnimatorComponent>(obj.first, GetActor(obj.first)->GetComponent<AnimatorComponent>().get());

        if (GetActor(obj.first)->GetComponent<ShadowSettings>())
            XMLObjectFile::writeUniqueComponent<ShadowSettings>(obj.first, GetActor(obj.first)->GetComponent<ShadowSettings>().get());
        
        if (GetActor(obj.first)->GetComponent<TilingSettings>())
            XMLObjectFile::writeUniqueComponent<TilingSettings>(obj.first, GetActor(obj.first)->GetComponent<TilingSettings>().get());


        AssetManager& assetMgr = AssetManager::getInstance();


        //write a referenced component (asset)
        if (GetActor(obj.first)->GetComponent<MeshComponent>()) {
            XMLObjectFile::writeReferenceComponent<MeshComponent>(obj.first, GetActor(obj.first)->GetComponent<MeshComponent>());
        }

        if (GetActor(obj.first)->GetComponent<MaterialComponent>())
            XMLObjectFile::writeReferenceComponent<MaterialComponent>(obj.first, GetActor(obj.first)->GetComponent<MaterialComponent>());

        if (GetActor(obj.first)->GetComponent<ShaderComponent>())
            XMLObjectFile::writeReferenceComponent<ShaderComponent>(obj.first, GetActor(obj.first)->GetComponent<ShaderComponent>());

        for (auto& script : GetActor(obj.first)->GetAllComponent<ScriptComponent>()) {
            XMLObjectFile::writeReferenceComponent<ScriptComponent>(obj.first, script->getBaseAsset());
        }

        GetActor(obj.first)->GetComponent<TransformComponent>()->GetPosition().print();
        XMLObjectFile::writeActorToCell(name, GetActor(obj.first), true);
    }
}

template<typename ComponentTemplate>
inline int XMLObjectFile::writeReferenceComponent(std::string name, Ref<Component> toWrite)
{
    std::string path = BuildPath("Game Objects/" + SceneGraph::getInstance().sceneFileName + "/" + name + ".xml", SearchPath::getInstance().GetEngineRoot()).string();
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


    std::string assetStringName = assetMgr.GetAssetName(toWrite);

    const char* assetName = assetStringName.c_str();

    //Add the cstr name as attribute
    componentElement->SetAttribute("name", assetName);
    const char* cStrIndex = "0";

    if (componentType == "ScriptComponent") {

        Ref<ScriptComponent> script = std::dynamic_pointer_cast<ScriptComponent>(SceneGraph::getInstance().GetActor(name)->GetComponent<ScriptComponent>());

        std::unordered_map<std::string, sol::object> pubRefMap = script->getPublicReferences();
        



        int index = 0;

        

        for (auto& pair : pubRefMap) {

            std::string strIndexName = "e" + std::to_string(index);
            const char* cStrIndex = strIndexName.c_str();
            float var = pair.second.as<float>();

            componentElement->SetAttribute(cStrIndex, var);
            index++;
        }



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

    
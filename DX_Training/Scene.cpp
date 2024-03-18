#include "stdafx.h"
#include "Scene.h"

Scene::Scene()
{
    // Create the FBX SDK memory manager object.
    // The SDK Manager allocates and frees memory
    // for almost all the classes in the SDK.
    _FBXManager = FbxManager::Create();

    // create an IOSettings object
    FbxIOSettings* ios = FbxIOSettings::Create(_FBXManager, IOSROOT);
    _FBXManager->SetIOSettings(ios);

    _scene = FbxScene::Create(_FBXManager, "");
}

Scene::~Scene()
{
    if (_FBXManager) 
        _FBXManager->Destroy();
}

void Scene::Draw(ComPtr<ID3D12GraphicsCommandList> commandList)
{
    _rootNode->Draw(commandList);
}

bool Scene::LoadScene(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList)
{
    bool lStatus;

    // Create an importer.
    FbxImporter* lImporter = FbxImporter::Create(_FBXManager, "");

    // Initialize the importer by providing a filename.
    bool lImportStatus = lImporter->Initialize(name.c_str(), -1, _FBXManager->GetIOSettings());

    if (!lImportStatus)
    {
        // Destroy the importer
        lImporter->Destroy();
        return false;
    }

    if (lImporter->IsFBX())
    {
        // Set the import states. By default, the import states are always set to 
        // true. The code below shows how to change these states.
        (*(_FBXManager->GetIOSettings())).SetBoolProp(IMP_FBX_MATERIAL, true);
        (*(_FBXManager->GetIOSettings())).SetBoolProp(IMP_FBX_TEXTURE, true);
        (*(_FBXManager->GetIOSettings())).SetBoolProp(IMP_FBX_LINK, true);
        (*(_FBXManager->GetIOSettings())).SetBoolProp(IMP_FBX_SHAPE, true);
        (*(_FBXManager->GetIOSettings())).SetBoolProp(IMP_FBX_GOBO, true);
        (*(_FBXManager->GetIOSettings())).SetBoolProp(IMP_FBX_ANIMATION, true);
        (*(_FBXManager->GetIOSettings())).SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
    }

    this->_scene = FbxScene::Create(this->_FBXManager, "myScene");

    // Import the scene
    lStatus = lImporter->Import(_scene);

    auto num = _scene->GetMemberCount();
    _rootNode = std::make_shared<SceneNode>(_scene->GetRootNode(), commandList);

    // Destroy the importer
    lImporter->Destroy();

    return lStatus;
}

#include "stdafx.h"

#include "Scene.h"

#include "Scene/SceneNode.h"
#include "Scene/Camera.h"
#include "Render/Heap.h"
#include "Render/DescriptorHeap.h"
#include "Volumes/FrustumVolume.h"

// Create the FBX SDK memory manager object.
// The SDK Manager allocates and frees memory
// for almost all the classes in the SDK.
FbxManager* Scene::_FBXManager = FbxManager::Create();

Scene::Scene()
{
    //_FBXManager = FbxManager::Create();

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

void Scene::Draw(ComPtr<ID3D12GraphicsCommandList> commandList, const FrustumVolume& frustum)
{
    _rootNode->Draw(commandList, frustum);
}

void Scene::DrawAABB(ComPtr<ID3D12GraphicsCommandList> commandList)
{
    _rootNode->DrawAABB(commandList);
}

void Scene::UploadTextures(ComPtr<ID3D12GraphicsCommandList> commandList, Heap& heap, DescriptorHeap& descriptorHeap)
{
    _rootNode->UploadTextures(commandList, heap, descriptorHeap);
}

bool Scene::LoadScene(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList)
{
    bool lStatus;

    // Create an importer.
    FbxImporter* lImporter = FbxImporter::Create(_FBXManager, "");

    // Initialize the importer by providing a filename.
    bool lImportStatus = lImporter->Initialize(name.c_str(), -1, _FBXManager->GetIOSettings());
    //lImporter->SetEmbeddingExtractionFolder("Res");

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

    this->_scene = FbxScene::Create(this->_FBXManager, "Scene");

    // Import the scene
    lStatus = lImporter->Import(_scene);

    auto num = _scene->GetMemberCount();
    auto n1= _scene->GetTextureCount();

    for (int i = 0; i < _scene->GetRootNode()->GetChild(0)->GetMaterialCount(); ++i)
        auto t = _scene->GetRootNode()->GetChild(0)->GetMaterial(i)->GetTypeName();
    
    _rootNode = std::make_shared<SceneNode>(_scene->GetRootNode(), commandList); // TODO: remove child(0)

    // Destroy the importer
    lImporter->Destroy();

    return lStatus;
}

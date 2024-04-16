#include "stdafx.h"

#include "Scene.h"

#include "DXObjects/Texture.h"
#include "Scene/SceneNode.h"
#include "Scene/Camera.h"
#include "Volumes/FrustumVolume.h"

// Create the FBX SDK memory manager object.
// The SDK Manager allocates and frees memory
// for almost all the classes in the SDK.
FbxManager* Scene::_FBXManager = FbxManager::Create();

Scene::Scene()
{
    // create an IOSettings object
    FbxIOSettings* ios = FbxIOSettings::Create(_FBXManager, IOSROOT);
    _FBXManager->SetIOSettings(ios);

    _scene = FbxScene::Create(_FBXManager, "");
    
    {
        HeapDescription desc;
        desc.SetHeapType(D3D12_HEAP_TYPE_DEFAULT);
        desc.SetHeapFlags(D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES);
        desc.SetSize(_32MB);
        desc.SetMemoryPoolPreference(D3D12_MEMORY_POOL_UNKNOWN);
        desc.SetCPUPageProperty(D3D12_CPU_PAGE_PROPERTY_UNKNOWN);
        desc.SetVisibleNodeMask(1);
        desc.SetCreationNodeMask(1);

        _texturesHeap.SetDescription(desc);
        _texturesHeap.Create();
    }

    {
        DescriptorHeapDescription desc;
        desc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        desc.SetNumDescriptors(32);
        desc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
        desc.SetNodeMask(1);

        _texturesDescHeap.SetDescription(desc);
        _texturesDescHeap.Create("Textures Descriptor Heap");
    }
}

Scene::~Scene()
{
    if (_FBXManager) 
    {
        _FBXManager->Destroy();
    }
}

void Scene::Draw(ComPtr<ID3D12GraphicsCommandList> commandList, const FrustumVolume& frustum)
{
    _rootNode->Draw(commandList, frustum);
}

void Scene::DrawAABB(ComPtr<ID3D12GraphicsCommandList> commandList)
{
    _rootNode->DrawAABB(commandList);
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

    _rootNode = std::make_shared<SceneNode>(_scene->GetRootNode(), commandList, this);

    // Destroy the importer
    lImporter->Destroy();

    return lStatus;
}

void Scene::_UploadTexture(Texture* texture, ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    _texturesHeap.PlaceResource(*texture);

    texture->SetDescriptorHeap(&_texturesDescHeap);
    texture->UploadToGPU(commandList);
}

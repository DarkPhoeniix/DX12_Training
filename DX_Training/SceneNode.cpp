#include "stdafx.h"

#include "SceneNode.h"

#include "Application.h"

using namespace DirectX;

SceneNode::SceneNode(FbxNode* node, ComPtr<ID3D12GraphicsCommandList> commandList)
{
    FbxMesh* mesh = node->GetChild(0)->GetMesh();
    auto n = node->GetChild(0)->GetName();

    size_t verticesCount = mesh->GetControlPointsCount();
    _rawVertexData.reserve(verticesCount);
    
    // Read vertices data
    //{
    //    FbxGeometryElementNormal* normalElement = mesh->GetElementNormal();
    //    if (normalElement)
    //    {
    //        //if (normalElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
    //        {
    //            for (int vertexIndex = 0; vertexIndex < verticesCount; vertexIndex++)
    //            {
    //                VertexData vertex;

    //                FbxDouble3 position = mesh->GetControlPointAt(vertexIndex);
    //                vertex.Position = { (float)position[0], (float)position[1], (float)position[2] };

    //                int normalIndex = 0;

    //                if (normalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
    //                    normalIndex = vertexIndex;
    //                if (normalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
    //                    normalIndex = normalElement->GetIndexArray().GetAt(vertexIndex);

    //                FbxVector4 norm = normalElement->GetDirectArray().GetAt(normalIndex);

    //                vertex.Normal = { (float)norm[0], (float)norm[1], (float)norm[2] };
    //                vertex.Color = { 0.5f, 0.5f, 0.5f };

    //                _rawVertexData.push_back(vertex);
    //            }
    //        }
    //    }
    //}

    // Read indices data
    {
        size_t polygonCount = mesh->GetPolygonCount();
        size_t indexCount = polygonCount * 3;
        _rawIndexData.resize(indexCount);

        FbxGeometryElementNormal* normalElement = mesh->GetElementNormal();
        for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
        {
            for (int vertexIndex = 0; vertexIndex < mesh->GetPolygonSize(polygonIndex); ++vertexIndex)
            {
                VertexData vertex;

                FbxDouble3 position = mesh->GetControlPointAt(mesh->GetPolygonVertex(polygonIndex, vertexIndex));
                vertex.Position = { (float)position[0], (float)position[1], (float)position[2] };

                int normalIndex = 0;

                if (normalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
                    normalIndex = mesh->GetPolygonVertex(polygonIndex, vertexIndex);
                if (normalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
                    normalIndex = normalElement->GetIndexArray().GetAt(mesh->GetPolygonVertex(polygonIndex, vertexIndex));

                FbxVector4 norm = normalElement->GetDirectArray().GetAt(normalIndex);

                vertex.Normal = { (float)norm[0], (float)norm[1], (float)norm[2] };
                vertex.Color = { 0.5f, 0.5f, 0.5f };

                _rawVertexData.push_back(vertex);
                _rawIndexData[polygonIndex * 3 + vertexIndex] = mesh->GetPolygonVertex(polygonIndex, vertexIndex);
            }
        }
    }

    // Read transform
    {
        XMMATRIX scale = XMMatrixScaling(node->LclScaling.Get()[0], node->LclScaling.Get()[1], node->LclScaling.Get()[2]);
        //XMMATRIX rotate = XMMatrixRot(node->LclRotation.Get()[0], node->LclScaling.Get()[1], node->LclScaling.Get()[2]);
        XMMATRIX translate = XMMatrixTranslation(node->LclTranslation.Get()[0], node->LclTranslation.Get()[1], node->LclTranslation.Get()[2]);

        _transform = translate * scale;
    }

    ComPtr<ID3D12Resource> vertexBuffer;
    ComPtr<ID3D12Resource> indexBuffer;
    _UploadData(commandList, &vertexBuffer, _rawVertexData.size(), sizeof(VertexData), _rawVertexData.data());
    _UploadData(commandList, &indexBuffer, _rawIndexData.size(), sizeof(WORD), _rawIndexData.data());
    _vertexBuffer = std::make_shared<Resource>();
    _indexBuffer = std::make_shared<Resource>();
    _vertexBuffer->SetResource(vertexBuffer);
    _indexBuffer->SetResource(indexBuffer);


    _VBO.BufferLocation = _vertexBuffer->OffsetGPU(0);
    _VBO.SizeInBytes = static_cast<UINT>(_rawVertexData.size() * sizeof(VertexData));
    _VBO.StrideInBytes = sizeof(VertexData);

    _IBO.BufferLocation = _indexBuffer->OffsetGPU(0);
    _IBO.Format = DXGI_FORMAT_R16_UINT;
    _IBO.SizeInBytes = static_cast<UINT>(_rawIndexData.size() * sizeof(WORD));

}

void SceneNode::Draw(ComPtr<ID3D12GraphicsCommandList> commandList)
{
    for (const std::shared_ptr<SceneNode> node : _childNodes)
        node->Draw(commandList);

    _DrawCurrentNode(commandList);
}

void SceneNode::_UploadData(ComPtr<ID3D12GraphicsCommandList> commandList, 
    ID3D12Resource** destinationResource,
    size_t numElements, 
    size_t elementSize, 
    const void* bufferData,
    D3D12_RESOURCE_FLAGS flags)
{
    auto device = Application::get().getDevice();

    size_t bufferSize = numElements * elementSize;

    CD3DX12_HEAP_PROPERTIES heapTypeDefault(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC bufferWithFlags = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);

    Helper::throwIfFailed(device->CreateCommittedResource(
        &heapTypeDefault,
        D3D12_HEAP_FLAG_NONE,
        &bufferWithFlags,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(destinationResource)));

    CD3DX12_HEAP_PROPERTIES heapTypeUpload(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC buffer = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);
    ID3D12Resource* intermediateResource;

    if (bufferData)
    {
        Helper::throwIfFailed(device->CreateCommittedResource(
            &heapTypeUpload,
            D3D12_HEAP_FLAG_NONE,
            &buffer,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&intermediateResource)));

        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = bufferData;
        subresourceData.RowPitch = bufferSize;
        subresourceData.SlicePitch = subresourceData.RowPitch;

        UpdateSubresources(commandList.Get(),
            *destinationResource, intermediateResource,
            0, 0, 1, &subresourceData);
    }
}

void SceneNode::_DrawCurrentNode(ComPtr<ID3D12GraphicsCommandList> commandList)
{
    // TODO: SceneNode::_DrawCurrentNode

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &_VBO);
    commandList->IASetIndexBuffer(&_IBO);

    commandList->DrawIndexedInstanced(static_cast<UINT>(_rawIndexData.size()), 1, 0, 0, 0);
}

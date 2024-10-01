#include "stdafx.h"

#include "EmptyObject.h"

namespace
{
    // TODO: duplicate from StaticObject.cpp - remove
    DirectX::XMMATRIX ParseTransformationMatrix(const Json::Value& transform)
    {
        // TODO: this func is ugly, rework later
        DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();

        int sz = transform["Transform"].size();
        Json::Value val;
        for (int i = 0; i < sz; ++i)
        {
            val = transform["Transform"][std::format("r{}", i).c_str()];
            std::stringstream iss(val.asCString());
            float value = 0;

            DirectX::XMFLOAT4 r;
            iss >> r.x >> r.y >> r.z >> r.w;

            matrix.r[i] = DirectX::XMLoadFloat4(&r);
        }

        return matrix;
    }
} // namespace unnamed

namespace SceneLayer
{
    void EmptyObject::Draw(Core::GraphicsCommandList& commandList, const FrustumVolume& frustum) const
    {
    }

    void EmptyObject::DrawAABB(Core::GraphicsCommandList& commandList) const
    {
    }

    void EmptyObject::LoadNode(const std::string& filepath, Core::GraphicsCommandList& commandList)
    {
        Base::LoadNode(filepath, commandList);

        // Open and read Json file
        std::ifstream in(filepath, std::ios_base::in | std::ios_base::binary);
        Json::Value root;
        in >> root;

        // Parse name
        _name = root["Name"].asCString();

        // Parse trsformation matrix
        _transform = ParseTransformationMatrix(root);
    }
} // namespace SceneLayer

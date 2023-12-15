#pragma once

#include "IComponent.h"

class Transformation : public IComponent
{
public:
    const std::string& GetType() const override;

    void SetTranslation(const DirectX::XMFLOAT3& translation);
    const DirectX::XMFLOAT3& GetTranslation() const;

private:
    DirectX::XMFLOAT3 _translation;
};

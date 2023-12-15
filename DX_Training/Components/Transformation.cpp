#include "stdafx.h"

#include "Transformation.h"

namespace
{
    const std::string type = "Transformation";
}

Register_Component(Transformation);

const std::string& Transformation::GetType() const
{
    return type;
}

void Transformation::SetTranslation(const DirectX::XMFLOAT3& translation)
{
    this->_translation = translation;
}

const DirectX::XMFLOAT3& Transformation::GetTranslation() const
{
    return _translation;;
}

#pragma once

#include "Interfaces/IEntity.h"

class Object : public IEntity
{
public:
    void SetName(const std::string& name);
    std::string GetName() const;

protected:
    std::string _name;
};

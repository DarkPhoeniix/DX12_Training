#pragma once

#include "Interfaces/IObject.h"

class Object : public IObject
{
public:
    void SetName(const std::string& name);
    std::string GetName() const;

protected:
    std::string _name;
};

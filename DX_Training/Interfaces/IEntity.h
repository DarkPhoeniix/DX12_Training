#pragma once

class IEntity
{
public:
    virtual ~IEntity();

    void SetName(const std::string& name);
    const std::string& GetName() const;

protected:
    std::string _name;
};

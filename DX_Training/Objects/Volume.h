#pragma once


struct Transformation
{

};

class Volume
{
public:
    std::shared_ptr<Transformation> GetAbsoluteTransform() const;
    std::shared_ptr<Transformation> GetRelativeTransform() const;

    void AddChild(std::shared_ptr<Volume> volume);
    std::vector<std::shared_ptr<Volume>> GetChildren() const;

    void AddEntity(std::shared_ptr<IObject> entity);
    void RemoveEntity(std::shared_ptr<IObject> entity);
    std::vector<std::shared_ptr<IObject>> GetEntities() const;

private:
    std::shared_ptr<Transformation> _tranform;

    std::shared_ptr<Volume> _parent;
    std::vector<std::shared_ptr<Volume>> _children;

    std::vector<std::shared_ptr<IObject>> _entities;
};

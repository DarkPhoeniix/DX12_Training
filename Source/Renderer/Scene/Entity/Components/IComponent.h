#pragma once

namespace scene
{
    class IComponent
    {
    public:
        IComponent() = default;
        IComponent(const std::string& name)
            : ComponentName(name)
        {
        }

        std::string ComponentName;
    };
} // namespace scene

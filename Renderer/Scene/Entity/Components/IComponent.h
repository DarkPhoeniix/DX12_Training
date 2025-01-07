#pragma once

namespace SceneLayer
{
    struct IComponent
    {
        IComponent() = default;
        IComponent(const std::string& name)
            : ComponentName(name)
        {
        }

        std::string ComponentName;
    };
} // namespace SceneLayer

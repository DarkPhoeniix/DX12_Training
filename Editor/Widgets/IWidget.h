#pragma once

namespace scene
{
    class Viewport;
}

namespace gui
{
    class IWidget
    {
    public:
        virtual ~IWidget() = default;

        virtual void Init();
        virtual void Destroy();

        virtual void Update();

    protected:
        scene::Viewport* _viewport;
    };
} // namespace gui

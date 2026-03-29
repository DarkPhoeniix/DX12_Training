
#include "RenderPass.h"

namespace rg
{
    template<typename PassData>
    RenderPass<PassData>::RenderPass(const std::string& name, RenderPassType type)
        : IRenderPass(name, type)
        , _setupFunc(nullptr)
        , _executeFunc(nullptr)
        , _data{}
    {
    }

    template<typename PassData>
    RenderPass<PassData>::RenderPass(const std::string& name, SetupFunc&& setup, ExecuteFunc&& execute, RenderPassType type)
        : IRenderPass(name, type)
        , _setupFunc(std::move(setup))
        , _executeFunc(std::move(execute))
        , _data{}
    {
    }

    template<typename PassData>
    const PassData& RenderPass<PassData>::GetData() const
    {
        return _data;
    }

    template<typename PassData>
    void RenderPass<PassData>::Setup(RenderPassBuilder& builder)
    {
        if (_setupFunc)
        {
            _setupFunc(builder);
        }
    }

    template<typename PassData>
    void RenderPass<PassData>::Execute(RenderContext& context, ITask* task)
    {
        if (_executeFunc)
        {
            _executeFunc(context, task);
        }
    }
} // namespace rg

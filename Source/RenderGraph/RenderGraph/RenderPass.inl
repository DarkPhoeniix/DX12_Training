
#include "RenderPass.h"

namespace rg
{
    template<typename PassData>
    RenderPass<PassData>::RenderPass(const std::string& name, RenderPassType type)
        : IRenderPass(name, type)
    {
    }

    template<typename PassData>
    const PassData& RenderPass<PassData>::GetData() const
    {
        return _data;
    }
} // namespace rg

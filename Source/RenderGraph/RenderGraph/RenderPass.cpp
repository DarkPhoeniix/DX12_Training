#include "RenderGraphPCH.h"

#include "RenderPass.h"

namespace rg
{
    IRenderPass::IRenderPass(const std::string& name, RenderPassType type)
        : _name(name), _type(type)
    {
    }

    RenderPassType IRenderPass::GetType() const
    {
        return _type;
    }
} // namespace rg

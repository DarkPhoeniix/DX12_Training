#include "RenderGraphPCH.h"

#include "RenderGraphResourceId.h"

namespace rg
{
    rg::RGResourceId::RGResourceId()
        : ID(InvalidID)
    {
    }

    rg::RGResourceId::RGResourceId(std::uint64_t id)
        : ID(id)
    {
    }

    void rg::RGResourceId::Invalidate()
    {
        ID = InvalidID;
    }

    bool rg::RGResourceId::IsValid() const
    {
        return ID != InvalidID;
    }
} // namespace rg

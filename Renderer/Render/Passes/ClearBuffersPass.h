#pragma once

#include "IRenderPass.h"

namespace render
{
    class ClearBuffersPass : public IRenderPass
    {
    public:
        // Inherited via IRenderPass
        void Initialize() override;
        void Destroy() override;

        void Execute() override;
    };
} // namespace render

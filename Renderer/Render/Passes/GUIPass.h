#pragma once

#include "IRenderPass.h"

namespace render
{
    class GUIPass : public IRenderPass
    {
    public:
        // Inherited via IRenderPass
        void Initialize() override;
        void Destroy() override;

        void Execute() override;
    };
} // namespace render

#pragma once

#include "IRenderPass.h"

class ClearBuffersPass : public IRenderPass
{
public:
    // Inherited via IRenderPass
    void Inititalize() override;
    void Destroy() override;

    void Execute() override;
};

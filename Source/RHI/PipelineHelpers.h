#pragma once

#include "Format.h"
#include "PipelineStates.h"

namespace Json
{
    class Value;
} // namespace Json

namespace rhi::internal
{
    PipelineStateType ParsePipelineType(const std::string& typeName);
    Json::Value ParseJson(const std::string& filepath);

    Format ParseFormat(const std::string& str);

    Blend ParseBlend(const std::string& str);
    BlendOpType ParseBlendOp(const std::string& str);
    ColorWriteEnable ParseColorWriteEnable(const std::string& str);
    LogicOp ParseLogicOp(const std::string& str);

    FillMode ParseFillMode(const std::string& str);
    CullMode ParseCullMode(const std::string& str);

    ComparisonFunc ParseComparisonFunc(const std::string& str);
    DepthWriteMask ParseDepthWriteMask(const std::string& str);
} // namespace rhi::internal

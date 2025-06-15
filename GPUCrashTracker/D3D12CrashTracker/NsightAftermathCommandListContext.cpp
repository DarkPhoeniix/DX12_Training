
#include "NsightAftermathCommandListContext.h"
#include "GFSDK_Aftermath.h"

#include <cassert>
#include <cstring>

namespace tracking
{
    NsightAftermathCommandListContext::NsightAftermathCommandListContext()
        : _commandList(nullptr)
        , _commandListCrashContext(nullptr)
    {
    }

    void NsightAftermathCommandListContext::Initialize(ID3D12GraphicsCommandList* commandList)
    {
        assert(commandList);
        if (commandList)
        {
            _commandList = commandList;
            // Create an Nsight Aftermath context handle for setting Aftermath event markers in this command list.
            AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_DX12_CreateContextHandle(_commandList, &_commandListCrashContext));
        }
    }

    void NsightAftermathCommandListContext::SetMarker(const char* marker)
    {
        assert(_commandListCrashContext);
        if (_commandListCrashContext)
        {
            // Set the marker in the command list context
            AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_SetEventMarker(_commandListCrashContext, marker, static_cast<uint32_t>(std::strlen(marker))));
        }
    }
} // namespace tracking


#include "NsightAftermathCommandListContext.h"

#include "../IGPUCrashTracker.h"

#include "GFSDK_Aftermath.h"

#include <cassert>
#include <cstring>

namespace tracking
{
    NsightAftermathCommandListContext::NsightAftermathCommandListContext(std::shared_ptr<IGPUCrashTracker> crashTracker)
        : ICommandListCrashContext(crashTracker)
        , _commandList(nullptr)
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
            // App is responsible for handling marker memory, and for resolving the memory at crash dump generation time.
            // The actual "const void* markerData" passed to Aftermath in this case can be any uniquely identifying value that the app can resolve to the marker data later.
            // For this sample, we will use this approach to generating a unique marker value:
            // We keep a ringbuffer with a marker history of the last c_markerFrameHistory frames (currently 4).
            UINT markerMapIndex = _crashTracker->GetMarkerFrameIndex() % IGPUCrashTracker::MarkerFrameHistory;
            auto& currentFrameMarkerMap = _crashTracker->GetMarkerMap()[markerMapIndex];
            // Take the index into the ringbuffer, multiply by 10000, and add the total number of markers logged so far in the current frame, +1 to avoid a value of zero.
            size_t markerID = markerMapIndex * 10000 + currentFrameMarkerMap.size() + 1;
            // This value is the unique identifier we will pass to Aftermath and internally associate with the marker data in the map.
            currentFrameMarkerMap[markerID] = marker;
            AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_SetEventMarker(_commandListCrashContext, (void*)markerID, 0));
        }
    }
} // namespace tracking

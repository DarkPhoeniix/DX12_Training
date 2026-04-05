
#include "RHI_PCH.h"

#include "GPUEvent.h"

namespace rhi
{
    GPUScopedEvent::GPUScopedEvent(CommandList* commandList, const char* name, std::uint8_t color)
        : _commandList(commandList)
    {
        _commandList->BeginEvent(name, color);
    }

    GPUScopedEvent::~GPUScopedEvent()
    {
        _commandList->EndEvent();
    }
} // namespace rhi

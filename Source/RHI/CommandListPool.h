
#include "CommandList.h"

namespace rhi
{
    class CommandListPool
    {
    public:
        virtual ~CommandListPool() = default;

        [[nodiscard]] virtual CommandList* AllocateCommandList(CommandListType type) = 0;
        virtual void FreeCommandList(CommandList* commandList) = 0;
    };
} // namespace rhi
